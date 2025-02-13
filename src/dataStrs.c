/*
 * This file is part of the PikaScript project.
 * http://github.com/pikastech/pikascript
 *
 * MIT License
 *
 * Copyright (c) 2021 lyon 李昂 liang6516@outlook.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "dataStrs.h"
#include "PikaPlatform.h"
#include "dataString.h"

Args* New_strBuff(void) {
    return New_args(NULL);
}

char* strsRemovePrefix(Args* buffs, char* inputStr, char* prefix) {
    int32_t size = strGetSize(inputStr);
    char* buff = args_getBuff(buffs, size);
    return strRemovePrefix(inputStr, prefix, buff);
}

char* strsGetDirectStr(Args* buffs, char* argPath) {
    char* directStr = NULL;
    directStr = strsCut(buffs, argPath, '"', '"');
    if (NULL != directStr) {
        return directStr;
    }
    directStr = strsCut(buffs, argPath, '\'', '\'');
    if (NULL != directStr) {
        return directStr;
    }
    return NULL;
}

char* strsAppend(Args* buffs, char* strOrigin, char* strToAppend) {
    int32_t size = strGetSize(strOrigin) + strGetSize(strToAppend);
    char* buff = args_getBuff(buffs, size);
    char* strOut = strCopy(buff, strOrigin);
    strAppend(strOut, strToAppend);
    return strOut;
}

char* strsGetLastToken(Args* buffs, char* argPath, char sign) {
    int32_t size = strGetSize(argPath);
    char* buff = args_getBuff(buffs, size);
    return strGetLastToken(buff, argPath, sign);
}

char* strsCut(Args* buffs, char* strIn, char startSign, char endSign) {
    int32_t size = strGetSize(strIn);
    char* buff = args_getBuff(buffs, size);
    return strCut(buff, strIn, startSign, endSign);
}

char* strsDeleteChar(Args* buffs, char* strIn, char ch) {
    int32_t size = strGetSize(strIn);
    return strDeleteChar(args_getBuff(buffs, size), strIn, ch);
}

static uint32_t getSizeOfFirstToken(char* str, char sign) {
    uint32_t size = strGetSize(str);
    for (uint32_t i = 0; i < size; i++) {
        if (str[i] == sign) {
            return i;
        }
    }
    return size;
}

char* strsGetFirstToken(Args* buffs, char* strIn, char sign) {
    int32_t size = getSizeOfFirstToken(strIn, sign);
    return strGetFirstToken(args_getBuff(buffs, size), strIn, sign);
}

char* strsPopToken(Args* buffs, char* tokens, char sign) {
    int32_t size = strGetSize(tokens);
    char* buff = args_getBuff(buffs, size);
    return strPopToken(buff, tokens, sign);
}

char* strsCopy(Args* buffs, char* source) {
    int32_t size = strGetSize(source);
    char* buff = args_getBuff(buffs, size);
    return strCopy(buff, source);
}

char* strsFormat(Args* buffs, uint16_t buffSize, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char* res = args_getBuff(buffs, buffSize);
    __platform_vsnprintf(res, buffSize, fmt, args);
    va_end(args);
    return res;
}

Arg* arg_strAppend(Arg* arg_in, char* str_to_append) {
    Args* buffs = New_strBuff();
    char* str_out = strsAppend(buffs, arg_getStr(arg_in), str_to_append);
    Arg* arg_out = arg_setStr(arg_in, "", str_out);
    arg_deinit(arg_in);
    args_deinit(buffs);
    return arg_out;
}

char* strsReplace(Args* buffs, char* orig, char* rep, char* with) {
    char* result;   // the return string
    char* ins;      // the next insert point
    char* tmp;      // varies
    int len_rep;    // length of rep (the string to remove)
    int len_with;   // length of with (the string to replace rep with)
    int len_front;  // distance between rep and end of last rep
    int count;      // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL;  // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);
    // count the number of replacements needed
    ins = orig;
    tmp = strstr(ins, rep);
    count = 0;
    while (tmp) {
        count++;
        ins = tmp + len_rep;
        tmp = strstr(ins, rep);
    }
    tmp = args_getBuff(buffs, strlen(orig) + (len_with - len_rep) * count + 1);
    result = tmp;
    if (NULL == result) {
        return NULL;
    }
    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep;  // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

static int32_t __getLineSize(char* str) {
    int i = 0;
    while (1) {
        if (str[i] == '\n') {
            return i;
        }
        i++;
    }
}

char* strsGetLine(Args* buffs, char* code) {
    int32_t lineSize = __getLineSize(code);
    char* line = args_getBuff(buffs, lineSize + 1);
    __platform_memcpy(line, code, lineSize);
    line[lineSize + 1] = 0;
    return line;
}
