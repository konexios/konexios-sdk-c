/*
  Copyright (C) 2011 Joseph A. Adams (joeyadams3.14159@gmail.com)
  All rights reserved.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "json/json.h"

#if defined(__arm__)
#if defined(ENABLE_ARM_TRACE)
//#include <cmsis_rtos.h>
//#include "trace.h"
#endif
#endif
#include <data/property.h>
#include <json/property_json.h>
#include <sys/mem.h>
#include <sys/reboot.h>
#if defined(__USE_STD__)
#include <assert.h>
#else
#define assert(...)
#endif
#include <math.h>
#include <debug.h>

#if defined(STATIC_JSON)
#include <data/static_alloc.h>
static_object_pool_type(JsonNode, KONEXIOS_MAX_JSON_OBJECTS)
#endif

#if defined(STATIC_JSON)
int JsonNode_object_alloc_size() {
    return static_alloc_size(JsonNode) / sizeof(JsonNode);
}
#endif

/* Sadly, strdup is not portable. */
char *json_strdup(const char *s) {
    SB out;
    sb_init(&out);
    int ret = sb_puts(&out, s);
    if (ret < 0) {
        return NULL;
    }
    sb_putc(&out, '\0');
    return out.start;
}

property_t  json_strdup_property(const char *str) {
    char *tmp = json_strdup(str);
    if ( !tmp ) return p_null;
    return p_json(tmp);
}

/*
 * Unicode helper functions
 *
 * These are taken from the ccan/charset module and customized a bit.
 * Putting them here means the compiler can (choose to) inline them,
 * and it keeps ccan/json from having a dependency.
 */

/*
 * Type for Unicode codepoints.
 * We need our own because wchar_t might be 16 bits.
 */
typedef uint32_t uchar_t;

/*
 * Validate a single UTF-8 character starting at @s.
 * The string must be null-terminated.
 *
 * If it's valid, return its length (1 thru 4).
 * If it's invalid or clipped, return 0.
 *
 * This function implements the syntax given in RFC3629, which is
 * the same as that given in The Unicode Standard, Version 6.0.
 *
 * It has the following properties:
 *
 *  * All codepoints U+0000..U+10FFFF may be encoded,
 *    except for U+D800..U+DFFF, which are reserved
 *    for UTF-16 surrogate pair encoding.
 *  * UTF-8 byte sequences longer than 4 bytes are not permitted,
 *    as they exceed the range of Unicode.
 *  * The sixty-six Unicode "non-characters" are permitted
 *    (namely, U+FDD0..U+FDEF, U+xxFFFE, and U+xxFFFF).
 */
static int utf8_validate_cz(const char *s)
{
	unsigned char c = (unsigned char) *s++;

	if (c <= 0x7F) {        /* 00..7F */
		return 1;
	} else if (c <= 0xC1) { /* 80..C1 */
		/* Disallow overlong 2-byte sequence. */
		return 0;
	} else if (c <= 0xDF) { /* C2..DF */
		/* Make sure subsequent byte is in the range 0x80..0xBF. */
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;

		return 2;
	} else if (c <= 0xEF) { /* E0..EF */
		/* Disallow overlong 3-byte sequence. */
		if (c == 0xE0 && (unsigned char)*s < 0xA0)
			return 0;

		/* Disallow U+D800..U+DFFF. */
		if (c == 0xED && (unsigned char)*s > 0x9F)
			return 0;

		/* Make sure subsequent bytes are in the range 0x80..0xBF. */
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;

		return 3;
	} else if (c <= 0xF4) { /* F0..F4 */
		/* Disallow overlong 4-byte sequence. */
		if (c == 0xF0 && (unsigned char)*s < 0x90)
			return 0;

		/* Disallow codepoints beyond U+10FFFF. */
		if (c == 0xF4 && (unsigned char)*s > 0x8F)
			return 0;

		/* Make sure subsequent bytes are in the range 0x80..0xBF. */
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;
		if (((unsigned char)*s++ & 0xC0) != 0x80)
			return 0;

		return 4;
	} else {                /* F5..FF */
		return 0;
	}
}

/* Validate a null-terminated UTF-8 string. */
static bool utf8_validate(const char *s)
{
	int len;

	for (; *s != 0; s += len) {
		len = utf8_validate_cz(s);
		if (len == 0)
			return false;
	}

	return true;
}

/*
 * Read a single UTF-8 character starting at @s,
 * returning the length, in bytes, of the character read.
 *
 * This function assumes input is valid UTF-8,
 * and that there are enough characters in front of @s.
 */
static int utf8_read_char(const char *s, uchar_t *out)
{
	const unsigned char *c = (const unsigned char*) s;

	assert(utf8_validate_cz(s));

	if (c[0] <= 0x7F) {
		/* 00..7F */
		*out = c[0];
		return 1;
	} else if (c[0] <= 0xDF) {
		/* C2..DF (unless input is invalid) */
		*out = ((uchar_t)c[0] & 0x1F) << 6 |
		       ((uchar_t)c[1] & 0x3F);
		return 2;
	} else if (c[0] <= 0xEF) {
		/* E0..EF */
		*out = ((uchar_t)c[0] &  0xF) << 12 |
		       ((uchar_t)c[1] & 0x3F) << 6  |
		       ((uchar_t)c[2] & 0x3F);
		return 3;
	} else {
		/* F0..F4 (unless input is invalid) */
		*out = ((uchar_t)c[0] &  0x7) << 18 |
		       ((uchar_t)c[1] & 0x3F) << 12 |
		       ((uchar_t)c[2] & 0x3F) << 6  |
		       ((uchar_t)c[3] & 0x3F);
		return 4;
	}
}

/*
 * Write a single UTF-8 character to @s,
 * returning the length, in bytes, of the character written.
 *
 * @unicode must be U+0000..U+10FFFF, but not U+D800..U+DFFF.
 *
 * This function will write up to 4 bytes to @out.
 */
static int utf8_write_char(uchar_t unicode, char *out)
{
	unsigned char *o = (unsigned char*) out;

	assert(unicode <= 0x10FFFF && !(unicode >= 0xD800 && unicode <= 0xDFFF));

	if (unicode <= 0x7F) {
		/* U+0000..U+007F */
		*o++ = (unsigned char)unicode;
		return 1;
	} else if (unicode <= 0x7FF) {
		/* U+0080..U+07FF */
		*o++ = (unsigned char)(0xC0 | unicode >> 6);
		*o++ = (unsigned char)(0x80 | (unicode & 0x3F));
		return 2;
	} else if (unicode <= 0xFFFF) {
		/* U+0800..U+FFFF */
		*o++ = (unsigned char)(0xE0 | unicode >> 12);
		*o++ = (unsigned char)(0x80 | (unicode >> 6 & 0x3F));
		*o++ = (unsigned char)(0x80 | (unicode & 0x3F));
		return 3;
	} else {
		/* U+10000..U+10FFFF */
		*o++ = (unsigned char)(0xF0 | unicode >> 18);
		*o++ = (unsigned char)(0x80 | (unicode >> 12 & 0x3F));
		*o++ = (unsigned char)(0x80 | (unicode >> 6 & 0x3F));
		*o++ = (unsigned char)(0x80 | (unicode & 0x3F));
		return 4;
	}
}

/*
 * Compute the Unicode codepoint of a UTF-16 surrogate pair.
 *
 * @uc should be 0xD800..0xDBFF, and @lc should be 0xDC00..0xDFFF.
 * If they aren't, this function returns false.
 */
static bool from_surrogate_pair(uint16_t uc, uint16_t lc, uchar_t *unicode)
{
	if (uc >= 0xD800 && uc <= 0xDBFF && lc >= 0xDC00 && lc <= 0xDFFF) {
		*unicode = 0x10000 + ((((uchar_t)uc & 0x3FF) << 10) | (lc & 0x3FF));
		return true;
	} else {
		return false;
	}
}

/*
 * Construct a UTF-16 surrogate pair given a Unicode codepoint.
 *
 * @unicode must be U+10000..U+10FFFF.
 */
static void to_surrogate_pair(uchar_t unicode, uint16_t *uc, uint16_t *lc)
{
	uchar_t n;

	assert(unicode >= 0x10000 && unicode <= 0x10FFFF);

	n = unicode - 0x10000;
	*uc = (uint16_t) (((n >> 10) & 0x3FF) | 0xD800);
	*lc = (uint16_t) ((n & 0x3FF) | 0xDC00);
}

static bool parse_value     (const char **sp, JsonNode        **out);
static bool parse_string    (const char **sp, property_t       *out);
static bool parse_number    (const char **sp, double           *out);
static bool parse_array     (const char **sp, JsonNode        **out);
static bool parse_object    (const char **sp, JsonNode        **out);
static bool parse_hex16     (const char **sp, uint16_t         *out);

static bool expect_literal  (const char **sp, const char *str);
static void skip_space      (const char **sp);

static int emit_value(SB *out, const JsonNode *node);
static void emit_value_indented     (SB *out, const JsonNode *node, const char *space, int indent_level);
static int emit_string(SB *out, const char *str);
void emit_number             (SB *out, double num);
static void emit_array              (SB *out, const JsonNode *array);
static void emit_array_indented     (SB *out, const JsonNode *array, const char *space, int indent_level);
static void emit_object             (SB *out, const JsonNode *object);
static void emit_object_indented    (SB *out, const JsonNode *object, const char *space, int indent_level);

static int write_hex16(char *out, uint16_t val);

static JsonNode *mknode(JsonTag tag);
static void append_node(JsonNode *parent, JsonNode *child);
static void prepend_node(JsonNode *parent, JsonNode *child);
static void append_member(JsonNode *object, property_t key, JsonNode *value);

/* Assertion-friendly validity checks */
static bool tag_is_valid(unsigned int tag);
static bool number_is_valid(const char *num);

JsonNode *json_decode(const char *json)
{
	const char *s = json;
	JsonNode *ret;

	skip_space(&s);
	if (!parse_value(&s, &ret))
		return NULL;

	skip_space(&s);
	if (*s != 0) {
		json_delete(ret);
		return NULL;
	}

	return ret;
}

property_t json_encode_property(const JsonNode *node) {
    char *alloc_string = json_stringify(node, NULL);
    if ( !alloc_string ) return p_null;
    property_t t = p_json(alloc_string);
    return t;
}

char *json_encode(const JsonNode *node)
{
	return json_stringify(node, NULL);
}

char *json_encode_string(const char *str)
{
	SB sb;
    if ( sb_init(&sb) < 0 ) return NULL;

    if ( emit_string(&sb, str) ) {
        sb_free(&sb);
        return NULL;
    }

	return sb_finish(&sb);
}

char *json_stringify(const JsonNode *node, const char *space)
{
	SB sb;
    if ( sb_init(&sb) < 0 ) return NULL;

	if (space != NULL)
		emit_value_indented(&sb, node, space, 0);
	else
		emit_value(&sb, node);

	return sb_finish(&sb);
}

void json_delete(JsonNode *node)
{
	if (node != NULL) {
		json_remove_from_parent(node);

		switch (node->tag) {
        case JSON_STRING: {
            property_free(&node->string_);
        } break;
			case JSON_ARRAY:
			case JSON_OBJECT:
			{
				JsonNode *child, *next;
				for (child = node->children.head; child != NULL; child = next) {
					next = child->next;
					json_delete(child);
				}
				break;
			}
			default:;
		}
#if defined(STATIC_JSON)
        static_free(JsonNode, node);
#else
		free(node);
#endif
    }
}

void json_delete_string(char *json_str) {
    SB out;
    out.start = json_str;
    sb_free(&out);
}

int weak_value_from_json(JsonNode *_node, property_t name, property_t *p) {
    if ( !_node ) return -1;

    JsonNode *tmp = json_find_member(_node, name);
    if ( ! tmp || tmp->tag != JSON_STRING ) return -1;
    property_weak_copy(p, tmp->string_);
    return 0;
}

bool json_validate(const char *json)
{
	const char *s = json;

	skip_space(&s);
	if (!parse_value(&s, NULL))
		return false;

	skip_space(&s);
	if (*s != 0)
		return false;

	return true;
}

JsonNode *json_find_element(JsonNode *array, int index)
{
	JsonNode *element;
	int i = 0;

	if (array == NULL || array->tag != JSON_ARRAY)
		return NULL;

	json_foreach(element, array) {
		if (i == index)
			return element;
		i++;
	}

	return NULL;
}

JsonNode *json_find_member(JsonNode *object, property_t name)
{
	JsonNode *member;

	if (object == NULL || object->tag != JSON_OBJECT)
		return NULL;

	json_foreach(member, object)
        if (property_cmp(&member->key, name) == 0)
			return member;

	return NULL;
}

JsonNode *json_first_child(const JsonNode *node)
{
	if (node != NULL && (node->tag == JSON_ARRAY || node->tag == JSON_OBJECT))
		return node->children.head;
	return NULL;
}

static void delayed_reboot(void* param) {
    // reboot();
	(void)param;
    reboot(0);
}
static JsonNode *mknode(JsonTag tag)
{
#if defined(STATIC_JSON)
    JsonNode *ret = static_allocator(JsonNode);
#else
    JsonNode *ret = (JsonNode*) calloc(1, sizeof(JsonNode));
#endif
    if (ret == NULL) {
#if defined(__arm__)
        #if defined(ENABLE_ARM_TRACE)
        trace(TRACE_CRITICAL, "JSON: Out of memory");
        xTimerStart(xTimerCreate("v_reboot", pdMS_TO_TICKS(20000), pdFALSE, NULL, delayed_reboot),0);
        #else
        DBG("JSON: Out of memory");
        #endif
#endif
    } else {
        memset(ret, 0x0, sizeof(JsonNode));
        ret->tag = tag;
    }
    return ret;
}

JsonNode *json_mknull(void)
{
	return mknode(JSON_NULL);
}

JsonNode *json_mkbool(bool b)
{
	JsonNode *ret = mknode(JSON_BOOL);
    if ( ret ) ret->bool_ = b;
	return ret;
}

static JsonNode *mkstring(property_t *s) {
	JsonNode *ret = mknode(JSON_STRING);
    if ( ret ) property_move(&ret->string_, s);
	return ret;
}

static JsonNode *mk_weak_property(property_t s) {
    JsonNode *ret = mknode(JSON_STRING);
    if ( ret ) property_weak_copy(&ret->string_, s);
	return ret;
}

JsonNode *json_mkstring(const char *s) {
    property_t p = json_strdup_property(s);
    if ( IS_EMPTY(p) ) return NULL;
    return mkstring(&p);
}

JsonNode *json_mkproperty(property_t *s) {
    return mkstring(s);
}

JsonNode *json_mk_weak_property(property_t s) {
    return mk_weak_property(s);
}

JsonNode *json_mknumber(double n)
{
	JsonNode *node = mknode(JSON_NUMBER);
    if ( node ) node->number_ = n;
	return node;
}

JsonNode *json_mkarray(void)
{
	return mknode(JSON_ARRAY);
}

JsonNode *json_mkobject(void)
{
	return mknode(JSON_OBJECT);
}

static void append_node(JsonNode *parent, JsonNode *child)
{
	child->parent = parent;
	child->prev = parent->children.tail;
	child->next = NULL;

	if (parent->children.tail != NULL)
		parent->children.tail->next = child;
	else
		parent->children.head = child;
	parent->children.tail = child;
}

static void prepend_node(JsonNode *parent, JsonNode *child) {
	child->parent = parent;
	child->prev = NULL;
	child->next = parent->children.head;

	if (parent->children.head != NULL)
		parent->children.head->prev = child;
	else
		parent->children.tail = child;
	parent->children.head = child;
}

static void append_member(JsonNode *object, property_t key, JsonNode *value) {
    if ( !value ) {
        DBG("%s : fail", __func__);
        return;
    }
    property_move(&value->key, &key);
	append_node(object, value);
}

void json_append_element(JsonNode *array, JsonNode *element) {
    if ( !element ) {
        DBG("%s : fail", __func__);
        return;
    }
	assert(array->tag == JSON_ARRAY);
	assert(element->parent == NULL);

	append_node(array, element);
}

void json_prepend_element(JsonNode *array, JsonNode *element) {
    if ( !element ) {
        DBG("%s : fail", __func__);
        return;
    }
	assert(array->tag == JSON_ARRAY);
	assert(element->parent == NULL);

	prepend_node(array, element);
}

int json_append_member(JsonNode *object, const property_t key, JsonNode *value) {
    if ( !value || IS_EMPTY(key) ) {
        DBG("%s : fail", __func__);
        return -1;
    }
    if (object->tag != JSON_OBJECT) return -1;
    if (value->parent != NULL) return -1;
    append_member(object, key, value);
    return 0;
}

void json_prepend_member(JsonNode *object, const property_t key, JsonNode *value) {
    if ( !value || IS_EMPTY(key) ) {
        DBG("%s : fail", __func__);
        return;
    }
	assert(object->tag == JSON_OBJECT);
	assert(value->parent == NULL);
    property_copy(&value->key, key);
	prepend_node(object, value);
}

// FIXME remove this
void json_remove_from_parent(JsonNode *node) {
	JsonNode *parent = node->parent;

	if (parent != NULL) {
		if (node->prev != NULL)
			node->prev->next = node->next;
		else
			parent->children.head = node->next;
		if (node->next != NULL)
			node->next->prev = node->prev;
		else
			parent->children.tail = node->prev;

        property_free(&node->key);

		node->parent = NULL;
		node->prev = node->next = NULL;
	}
}

static bool parse_value(const char **sp, JsonNode **out)
{
	const char *s = *sp;

	switch (*s) {
		case 'n':
			if (expect_literal(&s, "null")) {
                if (out) {
					*out = json_mknull();
                    if ( !*out ) return false;
                }
				*sp = s;
				return true;
			}
			return false;

		case 'f':
			if (expect_literal(&s, "false")) {
                if (out) {
					*out = json_mkbool(false);
                    if ( !*out ) return false;
                }
				*sp = s;
				return true;
			}
			return false;

		case 't':
			if (expect_literal(&s, "true")) {
                if (out) {
					*out = json_mkbool(true);
                    if ( !*out ) return false;
                }
				*sp = s;
				return true;
			}
			return false;

		case '"': {
            property_t str;
			if (parse_string(&s, out ? &str : NULL)) {
                if (out) {
                    *out = mkstring(&str);
                    if ( !*out ) return false;
                }
				*sp = s;
				return true;
			}
			return false;
		}

		case '[':
			if (parse_array(&s, out)) {
				*sp = s;
				return true;
			}
			return false;

		case '{':
			if (parse_object(&s, out)) {
				*sp = s;
				return true;
			}
			return false;

		default: {
			double num;
			if (parse_number(&s, out ? &num : NULL)) {
                if (out) {
					*out = json_mknumber(num);
                    if ( !*out ) return false;
                }
				*sp = s;
				return true;
			}
			return false;
		}
	}
}

static bool parse_array(const char **sp, JsonNode **out)
{
	const char *s = *sp;
	JsonNode *ret = out ? json_mkarray() : NULL;
	JsonNode *element;
    if ( !ret ) return false;

	if (*s++ != '[')
		goto failure;
	skip_space(&s);

	if (*s == ']') {
		s++;
		goto success;
	}

	for (;;) {
		if (!parse_value(&s, out ? &element : NULL))
			goto failure;
		skip_space(&s);

		if (out)
			json_append_element(ret, element);

		if (*s == ']') {
			s++;
			goto success;
		}

		if (*s++ != ',')
			goto failure;
		skip_space(&s);
	}

success:
	*sp = s;
	if (out)
		*out = ret;
	return true;

failure:
	json_delete(ret);
	return false;
}

static bool parse_object(const char **sp, JsonNode **out)
{
	const char *s = *sp;
	JsonNode *ret = out ? json_mkobject() : NULL;
    property_t key;
    JsonNode *value = NULL;
    if (!ret) return false;

	if (*s++ != '{')
		goto failure;
	skip_space(&s);

	if (*s == '}') {
		s++;
		goto success;
	}

	for (;;) {
		if (!parse_string(&s, out ? &key : NULL))
			goto failure;
		skip_space(&s);

		if (*s++ != ':')
			goto failure_free_key;
		skip_space(&s);

		if (!parse_value(&s, out ? &value : NULL))
			goto failure_free_key;
		skip_space(&s);

		if (out)
            append_member(ret, key, value);

		if (*s == '}') {
			s++;
			goto success;
		}

		if (*s++ != ',')
			goto failure;
		skip_space(&s);
	}

success:
	*sp = s;
	if (out)
		*out = ret;
	return true;

failure_free_key:
    if (out) {
        property_free(&key);
    }
failure:
	json_delete(ret);
	return false;
}

bool parse_string(const char **sp, property_t *out)
{
	const char *s = *sp;
    SB sb = {NULL, NULL, NULL};
	char throwaway_buffer[4];
		/* enough space for a UTF-8 character */
	char *b;

	if (*s++ != '"')
		return false;

	if (out) {
        if ( sb_init(&sb) < 0 ) return false;
        if ( sb_need(&sb, 4) < 0 ) goto failed;
		b = sb.cur;
	} else {
		b = throwaway_buffer;
	}

	while (*s != '"') {
		unsigned char c = (unsigned char) *s++;

		/* Parse next character, and write it to b. */
		if (c == '\\') {
			c = (unsigned char) *s++;
			switch (c) {
				case '"':
				case '\\':
				case '/':
					*b++ = (char) c;
					break;
				case 'b':
					*b++ = '\b';
					break;
				case 'f':
					*b++ = '\f';
					break;
				case 'n':
					*b++ = '\n';
					break;
				case 'r':
					*b++ = '\r';
					break;
				case 't':
					*b++ = '\t';
					break;
				case 'u':
				{
					uint16_t uc, lc;
					uchar_t unicode;

					if (!parse_hex16(&s, &uc))
						goto failed;

					if (uc >= 0xD800 && uc <= 0xDFFF) {
						/* Handle UTF-16 surrogate pair. */
						if (*s++ != '\\' || *s++ != 'u' || !parse_hex16(&s, &lc))
							goto failed; /* Incomplete surrogate pair. */
						if (!from_surrogate_pair(uc, lc, &unicode))
							goto failed; /* Invalid surrogate pair. */
					} else if (uc == 0) {
						/* Disallow "\u0000". */
						goto failed;
					} else {
						unicode = uc;
					}

					b += utf8_write_char(unicode, b);
					break;
				}
				default:
					/* Invalid escape */
					goto failed;
			}
		} else if (c <= 0x1F) {
			/* Control characters are not allowed in string literals. */
			goto failed;
		} else {
			/* Validate and echo a UTF-8 character. */
			int len;

			s--;
			len = utf8_validate_cz(s);
			if (len == 0)
				goto failed; /* Invalid UTF-8 character. */

			while (len--)
				*b++ = *s++;
		}

		/*
		 * Update sb to know about the new bytes,
		 * and set up b to write another character.
		 */
		if (out) {
			sb.cur = b;
            if ( sb_need(&sb, 4) < 0 ) goto failed;
			b = sb.cur;
		} else {
			b = throwaway_buffer;
		}
	}
	s++;

	if (out)
        *out = sb_finish_property(&sb);
	*sp = s;
	return true;

failed:
    if (out) {
		sb_free(&sb);
    }
	return false;
}

/*
 * The JSON spec says that a number shall follow this precise pattern
 * (spaces and quotes added for readability):
 *	 '-'? (0 | [1-9][0-9]*) ('.' [0-9]+)? ([Ee] [+-]? [0-9]+)?
 *
 * However, some JSON parsers are more liberal.  For instance, PHP accepts
 * '.5' and '1.'.  JSON.parse accepts '+3'.
 *
 * This function takes the strict approach.
 */
bool parse_number(const char **sp, double *out)
{
	const char *s = *sp;

	/* '-'? */
	if (*s == '-')
		s++;

	/* (0 | [1-9][0-9]*) */
	if (*s == '0') {
		s++;
	} else {
		if (!is_digit(*s))
			return false;
		do {
			s++;
		} while (is_digit(*s));
	}

	/* ('.' [0-9]+)? */
	if (*s == '.') {
		s++;
		if (!is_digit(*s))
			return false;
		do {
			s++;
		} while (is_digit(*s));
	}

	/* ([Ee] [+-]? [0-9]+)? */
	if (*s == 'E' || *s == 'e') {
		s++;
		if (*s == '+' || *s == '-')
			s++;
		if (!is_digit(*s))
			return false;
		do {
			s++;
		} while (is_digit(*s));
	}

	if (out)
		*out = strtod(*sp, NULL);

	*sp = s;
	return true;
}

static void skip_space(const char **sp)
{
	const char *s = *sp;
	while (is_space(*s))
		s++;
	*sp = s;
}

static int emit_value(SB *out, const JsonNode *node)
{
	assert(tag_is_valid(node->tag));
    int r = 0;
	switch (node->tag) {
		case JSON_NULL:
			sb_puts(out, "null");
			break;
		case JSON_BOOL:
			sb_puts(out, node->bool_ ? "true" : "false");
			break;
		case JSON_STRING:
            r = emit_string(out, P_VALUE(node->string_));
			break;
		case JSON_NUMBER:
			emit_number(out, node->number_);
			break;
		case JSON_ARRAY:
			emit_array(out, node);
			break;
		case JSON_OBJECT:
			emit_object(out, node);
			break;
		default:
			assert(false);
	}
    return r;
}

void emit_value_indented(SB *out, const JsonNode *node, const char *space, int indent_level)
{
	assert(tag_is_valid(node->tag));
	switch (node->tag) {
		case JSON_NULL:
			sb_puts(out, "null");
			break;
		case JSON_BOOL:
			sb_puts(out, node->bool_ ? "true" : "false");
			break;
		case JSON_STRING:
            emit_string(out, P_VALUE(node->string_));
			break;
		case JSON_NUMBER:
			emit_number(out, node->number_);
			break;
		case JSON_ARRAY:
			emit_array_indented(out, node, space, indent_level);
			break;
		case JSON_OBJECT:
			emit_object_indented(out, node, space, indent_level);
			break;
		default:
			assert(false);
	}
}

static void emit_array(SB *out, const JsonNode *array)
{
	const JsonNode *element;

	sb_putc(out, '[');
	json_foreach(element, array) {
		emit_value(out, element);
		if (element->next != NULL)
			sb_putc(out, ',');
	}
	sb_putc(out, ']');
}

static void emit_array_indented(SB *out, const JsonNode *array, const char *space, int indent_level)
{
	const JsonNode *element = array->children.head;
	int i;

	if (element == NULL) {
		sb_puts(out, "[]");
		return;
	}

	sb_puts(out, "[\n");
	while (element != NULL) {
		for (i = 0; i < indent_level + 1; i++)
			sb_puts(out, space);
		emit_value_indented(out, element, space, indent_level + 1);

		element = element->next;
		sb_puts(out, element != NULL ? ",\n" : "\n");
	}
	for (i = 0; i < indent_level; i++)
		sb_puts(out, space);
	sb_putc(out, ']');
}

static void emit_object(SB *out, const JsonNode *object)
{
	const JsonNode *member;

	sb_putc(out, '{');
	json_foreach(member, object) {
        emit_string(out, P_VALUE(member->key));
		sb_putc(out, ':');
		emit_value(out, member);
		if (member->next != NULL)
			sb_putc(out, ',');
	}
	sb_putc(out, '}');
}

static void emit_object_indented(SB *out, const JsonNode *object, const char *space, int indent_level)
{
	const JsonNode *member = object->children.head;
	int i;

	if (member == NULL) {
		sb_puts(out, "{}");
		return;
	}

	sb_puts(out, "{\n");
	while (member != NULL) {
		for (i = 0; i < indent_level + 1; i++)
			sb_puts(out, space);
        emit_string(out, P_VALUE(member->key));
		sb_puts(out, ": ");
		emit_value_indented(out, member, space, indent_level + 1);

		member = member->next;
		sb_puts(out, member != NULL ? ",\n" : "\n");
	}
	for (i = 0; i < indent_level; i++)
		sb_puts(out, space);
	sb_putc(out, '}');
}

int emit_string(SB *out, const char *str) {
	bool escape_unicode = false;
	const char *s = str;
	char *b;

	assert(utf8_validate(str));

	/*
	 * 14 bytes is enough space to write up to two
	 * \uXXXX escapes and two quotation marks.
	 */
    if ( sb_need(out, 14) ) return -1;
	b = out->cur;

	*b++ = '"';
	while (*s != 0) {
		unsigned char c = (unsigned char) *s++;

		/* Encode the next character, and write it to b. */
		switch (c) {
			case '"':
				*b++ = '\\';
				*b++ = '"';
				break;
			case '\\':
				*b++ = '\\';
				*b++ = '\\';
				break;
			case '\b':
				*b++ = '\\';
				*b++ = 'b';
				break;
			case '\f':
				*b++ = '\\';
				*b++ = 'f';
				break;
			case '\n':
				*b++ = '\\';
				*b++ = 'n';
				break;
			case '\r':
				*b++ = '\\';
				*b++ = 'r';
				break;
			case '\t':
				*b++ = '\\';
				*b++ = 't';
				break;
			default: {
				int len;

				s--;
				len = utf8_validate_cz(s);

				if (len == 0) {
					/*
					 * Handle invalid UTF-8 character gracefully in production
					 * by writing a replacement character (U+FFFD)
					 * and skipping a single byte.
					 *
					 * This should never happen when assertions are enabled
					 * due to the assertion at the beginning of this function.
					 */
					assert(false);
					if (escape_unicode) {
						strcpy(b, "\\uFFFD");
						b += 6;
					} else {
						*b++ = (char)0xEF;
						*b++ = (char)0xBF;
						*b++ = (char)0xBD;
					}
					s++;
				} else if (c < 0x1F || (c >= 0x80 && escape_unicode)) {
					/* Encode using \u.... */
					uint32_t unicode;

					s += utf8_read_char(s, &unicode);

					if (unicode <= 0xFFFF) {
						*b++ = '\\';
						*b++ = 'u';
						b += write_hex16(b, (uint16_t)unicode);
					} else {
						/* Produce a surrogate pair. */
						uint16_t uc, lc;
						assert(unicode <= 0x10FFFF);
						to_surrogate_pair(unicode, &uc, &lc);
						*b++ = '\\';
						*b++ = 'u';
						b += write_hex16(b, uc);
						*b++ = '\\';
						*b++ = 'u';
						b += write_hex16(b, lc);
					}
				} else {
					/* Write the character directly. */
					while (len--)
						*b++ = *s++;
				}

				break;
			}
		}

		/*
		 * Update *out to know about the new bytes,
		 * and set up b to write another encoded character.
		 */
		out->cur = b;
        if ( sb_need(out, 14) < 0 ) return -1;
		b = out->cur;
	}
	*b++ = '"';

	out->cur = b;
    return 0;
}

void emit_number(SB *out, double num)
{
	/*
	 * This isn't exactly how JavaScript renders numbers,
	 * but it should produce valid JSON for reasonable numbers
	 * preserve precision well enough, and avoid some oddities
	 * like 0.3 -> 0.299999999999999988898 .
	 */
	char buf[64];
	double intg;
	double fraction = modf(num, &intg);
    if ( fraction > 0.0 || fraction < 0.0 ) {
        sprintf(buf, "%.16g", num);
        if ( !number_is_valid(buf) ) {
            // there is no float implementation
            sprintf(buf, "%d.%03d", (int)intg, (int)(fraction * 1000));
        }
    } else
	    sprintf(buf, "%d", (int)num);

	if (number_is_valid(buf))
		sb_puts(out, buf);
	else
		sb_puts(out, "null");
}

static bool tag_is_valid(unsigned int tag)
{
	return (/* tag >= JSON_NULL && */ tag <= JSON_OBJECT);
}

static bool number_is_valid(const char *num)
{
	return (parse_number(&num, NULL) && *num == '\0');
}

static bool expect_literal(const char **sp, const char *str)
{
	const char *s = *sp;

	while (*str != '\0')
		if (*s++ != *str++)
			return false;

	*sp = s;
	return true;
}

/*
 * Parses exactly 4 hex characters (capital or lowercase).
 * Fails if any input chars are not [0-9A-Fa-f].
 */
static bool parse_hex16(const char **sp, uint16_t *out)
{
	const char *s = *sp;
	uint16_t ret = 0;
	uint16_t i;
	uint16_t tmp;
	char c;

	for (i = 0; i < 4; i++) {
		c = *s++;
		if (c >= '0' && c <= '9')
			tmp = (uint16_t) (c - '0');
		else if (c >= 'A' && c <= 'F')
			tmp = (uint16_t) (c - 'A' + 10);
		else if (c >= 'a' && c <= 'f')
			tmp = (uint16_t) (c - 'a' + 10);
		else
			return false;

		ret <<= (uint16_t)(4);
		ret += (uint16_t)tmp;
	}

	if (out)
		*out = ret;
	*sp = s;
	return true;
}

/*
 * Encodes a 16-bit number into hexadecimal,
 * writing exactly 4 hex chars.
 */
static int write_hex16(char *out, uint16_t val)
{
	const char *hex = "0123456789ABCDEF";

	*out++ = hex[(val >> 12) & 0xF];
	*out++ = hex[(val >> 8)  & 0xF];
	*out++ = hex[(val >> 4)  & 0xF];
	*out++ = hex[ val        & 0xF];

	return 4;
}

bool json_check(const JsonNode *node, char errmsg[256])
{
	#define problem(...) do { \
			if (errmsg != NULL) \
				snprintf(errmsg, 256, __VA_ARGS__); \
			return false; \
		} while (0)

    if ( !IS_EMPTY(node->key) && !utf8_validate(P_VALUE(node->key)))
		problem("key contains invalid UTF-8");

	if (!tag_is_valid(node->tag))
		problem("tag is invalid (%u)", node->tag);

	if (node->tag == JSON_BOOL) {
		if ((node->bool_ != false) && (node->bool_ != true))
			problem("bool_ is neither false (%d) nor true (%d)", (int)false, (int)true);
	} else if (node->tag == JSON_STRING) {
        if ( IS_EMPTY(node->string_) )
			problem("string_ is NULL");
        if (!utf8_validate(P_VALUE(node->string_)))
			problem("string_ contains invalid UTF-8");
	} else if (node->tag == JSON_ARRAY || node->tag == JSON_OBJECT) {
		JsonNode *head = node->children.head;
		JsonNode *tail = node->children.tail;

		if (head == NULL || tail == NULL) {
			if (head != NULL)
				problem("tail is NULL, but head is not");
			if (tail != NULL)
				problem("head is NULL, but tail is not");
		} else {
			JsonNode *child;
			JsonNode *last = NULL;

			if (head->prev != NULL)
				problem("First child's prev pointer is not NULL");

			for (child = head; child != NULL; last = child, child = child->next) {
				if (child == node)
					problem("node is its own child");
				if (child->next == child)
					problem("child->next == child (cycle)");
				if (child->next == head)
					problem("child->next == head (cycle)");

				if (child->parent != node)
					problem("child does not point back to parent");
				if (child->next != NULL && child->next->prev != child)
					problem("child->next does not point back to child");

                if (node->tag == JSON_ARRAY && !IS_EMPTY(child->key))
					problem("Array element's key is not NULL");
                if (node->tag == JSON_OBJECT && !IS_EMPTY(child->key))
					problem("Object member's key is NULL");

				if (!json_check(child, errmsg))
					return false;
			}

			if (last != tail)
				problem("tail does not match pointer found by starting at head and following next links");
		}
	}

	return true;

	#undef problem
}
