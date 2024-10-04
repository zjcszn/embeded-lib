#ifndef GBK_2_UTF8
#define GBK_2_UTF8

/*
 * 函数：
        int gbk_2_utf8(const char* str, int str_len, char* buf, int buf_len)
 * 描述：
 *      将GBK字符串转换成UTF8格式。
 * 参数：
 *      str     - GBK码字符串，这个字符串可以没有NULL结尾，这个时候需要str_len指出其长度。
 *      str_len - GBK码字符串长度，如果这个参数传入-1表示字符串是以NULL结尾的，函数自动计算其长度。
 *      buf     - 输出缓冲区，用来接收转换后的UTF8字符串。
 *      buf_len - 输出缓冲区长度，单位：字节。
 * 返回：
 *      返回填充到输出缓冲区的字节数，函数保证输出时UTF8字符不会因为缓冲区不够大而被截断。
 * 备注：
 *      只有当输出缓冲区足够大时才会在输出结果的末尾填入NULL字符，
 *      因此当输出缓冲区不够大时输出结果可能不会包括结尾NULL字符。
 *
 *      ASCII字符本身就是UTF8字符，因此占一个字节。
 *      请假设一个GBK字符转换成UTF8后需要三个字节。
 * 作者：
 *      novice@bbs.armfly.com
 * 
*/

extern int gbk_2_utf8(const char* str, int str_len, char* buf, int buf_len);

#endif /* GBK_2_UTF8 */
