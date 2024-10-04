/*
*web服务器例程
*本例程在非os环境下编写，占用内存很小
*可实现表单提交和文件传输等交互功能
*例程没有进行表单数据的处理,只实现了重启功能
*状态机的入口是handle_web_sm，由tcp_recv_thread线程调用
*
*/


#define DEBUG
#include "drivers.h"
#include "app.h"
#include "api.h"

#include "test.h"

#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"

#include "webserver.h"

#include "index_page.h"
#include "logo_gif.h"

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <rtw_byteorder.h>

#include <hal_intf.h>


int write_update_data(uint8_t *buff, int size)
{
	p_err_miss;
	return 0;
}

int erase_update_erea(uint32_t addr, uint32_t size)
{
	p_err_miss;
	return 0;
}

void write_update_data_finish()
{
	p_err_miss;
}


#if SUPPORT_WEBSERVER

#define SIZEOF_STR(X) (sizeof(X) - 1)

const char err_404_page[] = "HTTP/1.1 404 Object Not Found\r\n\
Server: WSUM-WebServer-v1.0\r\n\
Connection: close\r\n\
Content-Length: 75\r\n\
Content-Type: text/html\r\n\
\r\n\
<html><head><title>Error</title></head><body>Object Not Found</body></html>";

const char page_head[] = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\r\n\
<html xmlns=\"http://www.w3.org/1999/xhtml\">\r\n\
<head>\r\n\
<meta http-equiv=\"Content-Type\" content=\"text/html; charset=gb2312\" />\r\n\
<title></title>\r\n\
<style type=\"text/css\">\r\n\
<!--\r\n\
.STYLE16 {font-size: 16px}\r\n\
.STYLE18 {font-size: 24px; font-weight: bold; }\r\n\
-->\r\n\
</style>\r\n\
</head>\r\n\r\n";

const char http_text_head[] = "HTTP/1.1 200 OK\r\n\
Server: WSUM-WebServer-v1.0\r\n\
Connection: Keep-Alive\r\n\
Content-Type: text/html\r\n\
Content-Length: ";

const char http_gif_head[] = "HTTP/1.1 200 OK\r\n\
Server: WSUM-WebServer-v1.0\r\n\
Connection: Keep-Alive\r\n\
Content-Type: image/gif\r\n\
Content-Length: ";

const char http_body_head[] = "<body>\r\n\
<form id=\"form1\" name=\"form1\" method=\"post\" action=\"\">\r\n\
<center> \r\n\
<table width=\"500\" height=\"400\" border=\"0\" bordercolor=\"#000000\" bgcolor=\"#eeeeee\">\r\n";

const char http_body_head_update[] = "<body>\r\n\
<form action=\"update.html\" method=\"post\" enctype =\"multipart/form-data\" runat=\"server\">\r\n\
<center> \r\n\
<table width=\"500\" height=\"400\" border=\"0\" bordercolor=\"#000000\" bgcolor=\"#eeeeee\">\r\n";


const char http_body_tail[] = "</table>\r\n\
</center>\r\n\
</form>\r\n\
</body>\r\n\
</html>";


const char str_content_len[]  = "Content-Length";
const char str_get[]  = "GET";
const char str_post[]  = "POST";
const char str_line_end[]  = "\r\n";
const char str_sect_end[]  = "\r\n\r\n";

const char str_index_page[]  = "index.html";
const char str_device_info_page[]  = "device_info.html";
const char str_wifi_link_page[] = "wifi_link.html";
const char str_ip_addr_page[] = "ip_addr.html";
const char str_tcp_server_page[] = "tcp_server.html";
const char str_tcp_client_page[] = "tcp_client.html";	
const char str_remote_ctl_page[] = "remote_ctl.html";	
const char str_reset_page[] = "reset.html";
const char str_uart_page[] = "uart.html";
const char str_io_page[] = "io_ctrl.html";
const char str_update_page[] = "update.html";
const char str_cust_page[] = "cust_page.html";
const char str_login_page[] = "password.html";

const char str_logo_gif[]  = "logo.gif";

#define WEB_BUFF_SIZE 512
#define WEB_BUFF_MAX_VALID (WEB_BUFF_SIZE - 64)
char web_buff[WEB_BUFF_SIZE];

extern mutex_t socket_mutex;

WEB_CFG web_cfg;
/*
char *strstr(const char *haystack, const char *needle)
{
	int i = 0, needle_len = strlen(needle);
	
	while(haystack[i])
	{
		if(memcmp(haystack + i, needle, needle_len) == 0)
			return haystack + i;
		i++;
	}
	return 0;
}*/

void str_replace(char *buff, char *old, char *new)
{
	char *p_str;
	int len;
	len = strlen(old);
	if(len != strlen(new))
		return;
	do{
		p_str = strstr(buff, old);
		if(p_str)
		{
			memcpy(p_str, new, len);
		}
	}while(p_str);
}

int web_data_send(UCHAR s, const char *data, int len)
{
	int remain = len;
	char *p_data = (char*)data;
	int ret;
	web_cfg.web_sm = WEB_SM_SEND_DATA;
	if(s == 0xff)
		return len;
again:
	mutex_lock(socket_mutex);
	ret = send(s, p_data, remain, 0);
	mutex_unlock(socket_mutex);
	if(ret < 0){
		web_cfg.web_sm = WEB_SM_IDLE;
		p_err("e:%d", ret);
	}else{
		data += ret;
		remain -= ret;
		if(remain > 0){
			p_dbg("remain:%d", remain);
			goto again;
		}
	}
	return ret;
}


char *get_head_tag(const char *tag, const char *buff, int *len)
{
	int tag_len, buff_len;
	char *p_find, *p_tag_val_end;

	p_dbg_enter;
	
	if(!len || !tag || !buff)
		return 0;
	p_dbg("%x", len);
	*len = 0;
	p_dbg(tag);
	tag_len = strlen(tag);
	buff_len = strlen(buff);
	p_find = strstr(buff, tag);

	if(!p_find)
		return 0;

	p_find = p_find + tag_len;
	if((p_find >= (buff + buff_len)) || (*p_find != ':'))
		return 0;

	p_find++;
	while(*p_find == ' ')
		p_find++;
	p_tag_val_end = strchr(p_find, '\r');

	if(p_tag_val_end == 0)
		return 0;
	*len = p_tag_val_end - p_find;
	return p_find;
}


char *get_name(char *head, char *buff, int *len)
{
	int buff_len;
	char *p_find, *p_buff_end;
	p_dbg_enter;
	if(!len || !buff || !head)
		return 0;
	*len = 0;
	p_dbg(head);
	p_dbg(buff);
	buff_len = strlen(buff);
	p_buff_end = buff + buff_len;
	if(memcmp(buff, head, strlen(head)) != 0)
		return 0;

	while(*buff != '/' && (buff < p_buff_end))
		buff++;

	if(*buff != '/')
		return 0;

	buff++;
	
	p_find = buff;

	while(*buff != ' ' && (buff < p_buff_end))
		buff++;

	if(*buff != ' ')
		return 0;
	
	*len = buff - p_find;

#ifdef DEBUG
	*buff = 0;
	p_dbg("name:%s\n", p_find);
	*buff = ' ';
#endif

	return p_find;
}


char *get_body_tag(const char *tag, const char *buff, int *len)
{
	int tag_len, buff_len;
	char *p_find, *p_tag_val_end;
	if(!len || !tag || !buff)
		return 0;

	p_dbg_enter;
	*len = 0;
	tag_len = strlen(tag);
	buff_len = strlen(buff);
	p_find = strstr(buff, tag);
	if(!p_find){

		p_err("%s not in %s", tag, buff);
		return 0;
	}
	if((p_find >= (buff + buff_len)) || (p_find[tag_len] != '='))
	{
		p_err("? %c", p_find[tag_len]);
		p_dbg("%s", p_find);
		return 0;
	}
	p_find += tag_len + 1;
	
	p_tag_val_end = strchr(p_find, '&');
	if(p_tag_val_end == 0){
		p_tag_val_end = strchr(p_find, '\r');
		if(!p_tag_val_end)
		{
			p_tag_val_end = p_find + strlen(p_find);

			p_dbg("find end by '0':%s", p_find);
		}else
			p_dbg("find end by 'r':%s", p_find);
	}else
		p_dbg("find end by '&':%s", p_find);
	
	*len = p_tag_val_end - p_find;
	p_dbg("len:%d", *len);
	if(p_tag_val_end == p_find)
		return 0;
	return p_find;
}


int get_post_tag_value(const char *tag, const char *buff, char *str_val)
{
	int len;
	char *ret, tmp[64];
	if(str_val)
		str_val[0] = 0;
	if(!buff)
		return -1;
	ret = get_body_tag(tag, buff, &len);
	if(ret && len && (len < 64))
	{
		memcpy(tmp, ret, len); 
		tmp[len] = 0;
		if(str_val)
		{
			memcpy(str_val, tmp, len + 1);
			return 0;
		}
		else
			return atoi(tmp);
	}
	
	return -1;
}


int mem_move(void *start, void *end, int mv_len, int direct)
{
	int len; 
	char *p_start = (char*)start;
	char *p_end = (char*)end;
	if(!start || !end || !mv_len)
		return -1;
	if(start >= end)
		return -1;
	len = p_end - p_start + 1;

	if(direct)
	{
		while(len--)
		{
			*(p_end + mv_len) = *p_end;
			p_end--;
		}
		
	}else{
	
		while(len--)
		{
			*(p_start - mv_len)= *(p_start);
			p_start++;
		}
	}
	
	return 0;
}


//<input name="ip_gw" type="text" id="ip_gw" maxlength="15" />
int web_body_add_input_str(char *buff, const char *tag, const char *str)
{
	int buff_len, str_len;
	char *p_tag_entry, *p_tag_entry_end;
	p_dbg("enter %s\n", __FUNCTION__);
	if(!buff || !tag || !str)
		return -1;
	buff_len = strlen(buff);
	p_tag_entry = strstr(buff, tag);
	if(p_tag_entry == NULL || (buff > p_tag_entry))
		return -1;
	str_len = strlen(str);
	while(buff < p_tag_entry)
	{
		if(*p_tag_entry != '<')
			p_tag_entry--;
		else
			break;
	}
	if(*p_tag_entry != '<')
		return -1;

	p_tag_entry_end = p_tag_entry;
	while(p_tag_entry_end < (buff + buff_len))
	{
		if(*p_tag_entry_end != '>')
			p_tag_entry_end++;
		else
			break;
	}
	if(*p_tag_entry_end != '>' || *(p_tag_entry_end - 1) != '/')
		return -1;

	if(p_tag_entry_end <= p_tag_entry)
		return -1;

	if(mem_move(p_tag_entry_end - 1, &buff[buff_len] , str_len, 1) == 0)  //最后0也一起移
		memcpy(p_tag_entry_end - 1, str, str_len);
	else
		return -1;

//	p_dbg("add %d, new size %d\n", str_len, strlen(buff));

	return 0;
}

int web_body_add_option_str(char *buff, const char *tag, const char *str)
{
	int buff_len, str_len;
	char *p_tag_entry, *p_tag_entry_end;
	p_dbg("enter %s\n", __FUNCTION__);
	if(!buff || !tag || !str)
		return -1;
	buff_len = strlen((const char*)buff);
	p_tag_entry = strstr(buff, tag);
	if(p_tag_entry == NULL || (buff > p_tag_entry))
		return -1;
	str_len = strlen((const char*)str);
	while(buff < p_tag_entry)
	{
		if(*p_tag_entry != '<')
			p_tag_entry--;
		else
			break;
	}
	if(*p_tag_entry != '<')
		return -1;

	p_tag_entry_end = p_tag_entry;
	while(p_tag_entry_end < (buff + buff_len))
	{
		if(*p_tag_entry_end != '>')
			p_tag_entry_end++;
		else
			break;
	}
	if(*p_tag_entry_end != '>')
		return -1;

	if(p_tag_entry_end <= p_tag_entry)
		return -1;

	if(mem_move(p_tag_entry_end, &buff[buff_len] , str_len, 1) == 0)  //最后0也一起移
		memcpy(p_tag_entry_end, str, str_len);
	else
		return -1;

	p_dbg("add %d, new size %d\n", str_len, strlen((const char*)buff));

	return 0;
}


const char local_text1[] = "<tr>\r\n\
    <td  height=\"70\" colspan=\"3\"><div align=\"center\"><span class=\"STYLE18\">";

const char local_text2[] = "</span></div><hr /></td>\r\n\
  	</tr>\r\n";

void make_sub_page_title(char *name)
{
	sprintf(web_buff, "%s%s%s", local_text1, name, local_text2);
}

const char local_text3[] = "<tr>\r\n\
<td  height=\"30\" colspan=\"3\"> <span class=\"STYLE16\">\r\n\
<label></label>\r\n\
</span><span class=\"STYLE16\">\r\n\
<label>        </label>\r\n\
</span>\r\n\
<label>\r\n\
<div align=\"center\"><span class=\"STYLE16\">\r\n\
<input name=\"link_mode\" type=\"radio\" value=\"1\" />\r\n\
AP  \r\n\
<input name=\"link_mode\" type=\"radio\" value=\"0\" />\r\n\
STATION\r\n\
</span></div>\r\n\
</label>\r\n\
<span class=\"STYLE16\"></span></td>\r\n\
</tr>\r\n";

const char local_text4[]  = "<tr>\r\n\
<td width=\"38%\" height=\"30\"><div align=\"right\"><span class=\"STYLE16\">名称 :</span></div></td>\r\n\
<td width=\"38%\"><div align=\"left\"><span class=\"STYLE16\">\r\n\
<input name=\"link_ssid\" type=\"text\" id=\"link_ssid\" maxlength=\"32\"/>\r\n\
</span></div></td>\r\n\
<td width=\"24%\">&nbsp;</td>\r\n\
</tr>\r\n";


const char local_text5[]  = "<tr>\r\n\
<td height=\"30\"><div align=\"right\"><span class=\"STYLE16\">密码 :</span></div></td>\r\n\
<td><div align=\"left\"><span class=\"STYLE16\">\r\n\
<input name=\"link_key\" type=\"text\" id=\"link_key\" maxlength=\"13\"/>\r\n\
</span></div></td>\r\n\
<td>&nbsp;</td>\r\n\
</tr>\r\n";


const char local_text6[]  = "<tr>\r\n\
<td height=\"30\"><span class=\"STYLE16\">\r\n\
<div align=\"right\">频道 :</div>\r\n\
</label></span></td>\r\n\
<td><span class=\"STYLE16\">\r\n\
<label></label>\r\n\
<label></label>\r\n\
<div align=\"left\">\r\n\
<select name=\"link_channel\" size=\"1\" id=\"link_channel\">\r\n";


const char local_text6_1[]  = "<option value=\"1\">1</option>\r\n\
<option value=\"2\">2</option>\r\n\
<option value=\"3\">3</option>\r\n\
<option value=\"4\">4</option>\r\n\
<option value=\"5\">5</option>\r\n\
<option value=\"6\">6</option>\r\n\
<option value=\"7\">7</option>\r\n\
<option value=\"8\">8</option>\r\n\
<option value=\"9\">9</option>\r\n\
<option value=\"10\">10</option>\r\n\
<option value=\"11\">11</option>\r\n\
<option value=\"12\">12</option>\r\n\
<option value=\"13\">13</option>\r\n\
<option value=\"14\">14</option>\r\n";


const char local_text6_2[]  = "</select>\r\n\
</div>\r\n\
</span></td>\r\n\
<td><span class=\"STYLE16\">\r\n\
<label></label>\r\n\
<label></label>\r\n\
</span></td>\r\n\
</tr>\r\n";


const char local_text7_1[]  = "<tr>\r\n\
<td height=\"30\"><span class=\"STYLE16\">\r\n\
<div align=\"right\">加密 :</div>\r\n\
</span></td>\r\n\
<td><span class=\"STYLE16\">\r\n\
<label></label>\r\n\
<label></label>\r\n\
<div align=\"left\">\r\n";

const char local_text7_2[]  = "<select name=\"link_security\" size=\"1\" id=\"link_security\">\r\n\
<option value=\"0\">NONE</option>\r\n\
<option value=\"1\">WEP</option>\r\n\
<option value=\"2\">WPA+TKIP</option>\r\n\
<option value=\"3\">WPA2+AES</option>\r\n\
</select>\r\n";


const char local_text7_3[]   = "</div>\r\n\
</span></td>\r\n\
<td><span class=\"STYLE16\">\r\n\
<label></label>\r\n\
<label></label>\r\n\
</span></td>\r\n\
</tr>\r\n";

const char local_text8[]  ="<tr>\r\n\
<td colspan=\"2\"><div align=\"center\"><span class=\"STYLE17\"><span class=\"STYLE16\">\r\n\
<input name=\"bt_save\" type=\"submit\" id=\"bt_save\" value=\"确定\" />\r\n\
</span></span></div></td>\r\n\
</tr>\r\n";
#if 0
"<tr>\r\n\
<td height=\"98\" colspan=\"3\"><span class=\"STYLE16\">\r\n\
<label>\r\n\
<div align=\"right\">\r\n\
<div align=\"center\"><span class=\"STYLE16\">\r\n\
<input name=\"bt_save\" type=\"submit\" id=\"bt_save\" value=\"保存\" />\r\n\
</span></div>\r\n\
</div>\r\n\
<span class=\"STYLE16\"></span><span class=\"STYLE16\">\r\n\
</label>\r\n\
</span><span class=\"STYLE16\">\r\n\
<label></label>\r\n\
</span></td>\r\n\
</tr>\r\n";
#endif

const char local_text_linkmode_ap[]  = "input name=\"link_mode\" type=\"radio\" value=\"1\"";
const char local_text_linkmode_sta[]  = "input name=\"link_mode\" type=\"radio\" value=\"0\"";
const char local_text_checked[]  = " checked=\"checked\" ";

const char local_text_linkssid[]  = "<input name=\"link_ssid\" type=\"text\" id=\"link_ssid\" maxlength=\"32\"/>";
const char local_text_linkkey[]  = "<input name=\"link_key\" type=\"text\" id=\"link_key\" maxlength=\"13\"/>";



int wifi_link_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	char tmp[64];
	int ret;
	if(_sub_step != -1)
		step = _sub_step;

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head));
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head));
			return ret;
		case 2:
			step = 3;
			make_sub_page_title("WIFI配置");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 4;
			memcpy(web_buff, local_text3, SIZEOF_STR(local_text3));
			web_buff[SIZEOF_STR(local_text3)] = 0;
			if(web_cfg.link.mode)
			{
				web_body_add_input_str(web_buff, 
				local_text_linkmode_ap, 
				local_text_checked);
			}else{
				web_body_add_input_str(web_buff, 
				local_text_linkmode_sta, 
				local_text_checked);
			}
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 4:
		{
			step = 5;
			memcpy(web_buff, local_text4, SIZEOF_STR(local_text4));
			web_buff[SIZEOF_STR(local_text4)] = 0;

			sprintf(tmp, " value=\"%s\" ", web_cfg.link.essid);

			web_body_add_input_str(web_buff, 
				local_text_linkssid, 
				tmp); 
  			ret = web_data_send(s, web_buff, strlen(web_buff));
		}
			return ret;
		case 5:
			step = 6;
			memcpy(web_buff, local_text5, SIZEOF_STR(local_text5));
			web_buff[SIZEOF_STR(local_text5)] = 0;

			sprintf(tmp, " value=\"%s\" ", web_cfg.link.key);

			web_body_add_input_str(web_buff, 
				local_text_linkkey, 
				tmp); 
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 6:
			step = 7;
			memcpy(web_buff, local_text6, SIZEOF_STR(local_text6));
			web_buff[SIZEOF_STR(local_text6)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 7:
			step = 8;
			memcpy(web_buff, local_text6_1, SIZEOF_STR(local_text6_1));
			web_buff[SIZEOF_STR(local_text6_1)] = 0;

			sprintf((char*)tmp, "option value=\"%s\"", "auto");

			web_body_add_option_str(web_buff, tmp, " selected=\"selected\"");

  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 8:
			step = 9;
			memcpy(web_buff, local_text6_2, SIZEOF_STR(local_text6_2));
			web_buff[SIZEOF_STR(local_text6_2)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 9:
			step = 10;
			//assert(SIZEOF_STR(local_text7_1) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, local_text7_1, SIZEOF_STR(local_text7_1));
			web_buff[SIZEOF_STR(local_text7_1)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 10:
			step = 11;
			//assert(SIZEOF_STR(local_text7_2) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, local_text7_2, SIZEOF_STR(local_text7_2));
			web_buff[SIZEOF_STR(local_text7_2)] = 0;

			sprintf((char*)tmp, "option value=\"%s\"", "auto");

			web_body_add_option_str(web_buff, tmp, " selected=\"selected\"");
			
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 11:
			step = 12;
			//assert(SIZEOF_STR(local_text7_3) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, local_text7_3, SIZEOF_STR(local_text7_3));
			web_buff[SIZEOF_STR(local_text7_3)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 12:
			step = 13;
			assert(SIZEOF_STR(local_text8) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, local_text8, SIZEOF_STR(local_text8));
			web_buff[SIZEOF_STR(local_text8)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;	
		case 13:
			step = 14;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}


const char ip_addr_text1[]  = "<tr>\r\n\
<td width=\"31%\" height=\"28\"><div align=\"right\"><span class=\"STYLE16\">IP 地址:</span></div></td>\r\n\
<td width=\"69%\"><div align=\"left\"><span class=\"STYLE16\">\r\n\
<input name=\"ip_ipaddr\" type=\"text\" id=\"ip_ipaddr\" maxlength=\"15\"/>\r\n\
</span></div></td>\r\n\
</tr>\r\n";

const char ip_addr_text2[]  = "<tr>\r\n\
<td width=\"31%\" height=\"28\"><div align=\"right\"><span class=\"STYLE16\">网关地址:</span></div></td>\r\n\
<td width=\"69%\"><div align=\"left\"><span class=\"STYLE16\">\r\n\
<input name=\"ip_gw\" type=\"text\" id=\"ip_gw\" maxlength=\"15\"/>\r\n\
</span></div></td>\r\n\
</tr>\r\n";

const char ip_addr_text3[]  = "<tr>\r\n\
<td width=\"31%\" height=\"28\"><div align=\"right\"><span class=\"STYLE16\">子网掩码:</span></div></td>\r\n\
<td width=\"69%\"><div align=\"left\"><span class=\"STYLE16\">\r\n\
<input name=\"ip_mask\" type=\"text\" id=\"ip_mask\" maxlength=\"15\"/>\r\n\
</span></div></td>\r\n\
</tr>\r\n";

const char ip_addr_text4[]  = "<tr>\r\n\
<td width=\"31%\" height=\"28\"><div align=\"right\"><span class=\"STYLE16\">DNS地址:</span></div></td>\r\n\
<td width=\"69%\"><div align=\"left\"><span class=\"STYLE16\">\r\n\
<input name=\"ip_dns\" type=\"text\" id=\"ip_dns\" maxlength=\"15\"/>\r\n\
</span></div></td>\r\n\
</tr>\r\n";

const char ip_addr_text5[]  = "<tr>\r\n\
    <td colspan=\"2\"><div align=\"center\"><span class=\"STYLE16\">\r\n\
    </span></div>      <span class=\"STYLE16\"><label>\r\n\
      <div align=\"center\">\r\n\
        <input name=\"ip_auto\" type=\"checkbox\" id=\"ip_auto\" value=\"1\" />\r\n\
        自动分配</div>\r\n\
      </label>\r\n\
      <label></label>\r\n\
        </span></td>\r\n\
    </tr>\r\n";

const char ip_text_ipaddr[]  = "<input name=\"ip_ipaddr\" type=\"text\" id=\"ip_ipaddr\" maxlength=\"15\"/>";
const char ip_text_mask[]  = "<input name=\"ip_mask\" type=\"text\" id=\"ip_mask\" maxlength=\"15\"/>";
const char ip_text_gw[]  = "<input name=\"ip_gw\" type=\"text\" id=\"ip_gw\" maxlength=\"15\"/>";
const char ip_text_dns[]  = "<input name=\"ip_dns\" type=\"text\" id=\"ip_dns\" maxlength=\"15\"/>";
const char ip_text_auto[]  = "<input name=\"ip_auto\" type=\"checkbox\" id=\"ip_auto\" value=\"1\" />";

int ip_addr_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	char tmp[64];
	uint32_t dns_server;
	//unsigned char ip_addr[4];
	int ret;
	if(_sub_step != -1)
		step = _sub_step;

	

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head));
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head));
			return ret;
		case 2:
			step = 3;
			make_sub_page_title("IP地址配置");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 4;
			memcpy(web_buff, ip_addr_text1, SIZEOF_STR(ip_addr_text1));
			web_buff[SIZEOF_STR(ip_addr_text1)] = 0;
			 //uip_gethostaddr(&ip_addr);
			//sprintf(tmp, " value=\"%u.%u.%u.%u\" ", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
			sprintf(tmp, " value=\"%u.%u.%u.%u\" ", ip4_addr1(&p_netif->ip_addr.addr),  
			ip4_addr2(&p_netif->ip_addr.addr), 
			ip4_addr3(&p_netif->ip_addr.addr), 
			ip4_addr4(&p_netif->ip_addr.addr));
			web_body_add_input_str(web_buff, 
				ip_text_ipaddr, 
				tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 4:
			step = 5;
			memcpy(web_buff, ip_addr_text2, SIZEOF_STR(ip_addr_text2));
			web_buff[SIZEOF_STR(ip_addr_text2)] = 0;
			 //uip_getdraddr(&ip_addr);
			//sprintf(tmp, " value=\"%u.%u.%u.%u\" ", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
			//web_body_add_input_str(web_buff, 
			//	ip_text_gw, 
			//	tmp);
			sprintf(tmp, " value=\"%u.%u.%u.%u\" ", ip4_addr1(&p_netif->gw.addr),
			ip4_addr2(&p_netif->gw.addr),
			ip4_addr3(&p_netif->gw.addr),
			ip4_addr4(&p_netif->gw.addr));
			web_body_add_input_str(web_buff, 
				ip_text_gw, 
				tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 5:
			step = 6;
			memcpy(web_buff, ip_addr_text3, SIZEOF_STR(ip_addr_text3));
			web_buff[SIZEOF_STR(ip_addr_text3)] = 0;
			// uip_getnetmask(&ip_addr);
			//sprintf(tmp, " value=\"%u.%u.%u.%u\" ", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
			sprintf(tmp, " value=\"%u.%u.%u.%u\" ", ip4_addr1(&p_netif->netmask.addr),
			ip4_addr2(&p_netif->netmask.addr),
			ip4_addr3(&p_netif->netmask.addr),
			ip4_addr4(&p_netif->netmask.addr));
			web_body_add_input_str(web_buff, 
				ip_text_mask, 
				tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 6:
			step = 7;
			dns_server = dns_getserver(0).addr;
			memcpy(web_buff, ip_addr_text4, SIZEOF_STR(ip_addr_text4));
			web_buff[SIZEOF_STR(ip_addr_text4)] = 0;
			sprintf(tmp, " value=\"%u.%u.%u.%u\" ", ip4_addr1(&dns_server),
				ip4_addr2(&dns_server),
				ip4_addr3(&dns_server),
				ip4_addr4(&dns_server));
			web_body_add_input_str(web_buff, 
				ip_text_dns, 
				tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 7:
			step = 8;
			memcpy(web_buff, ip_addr_text5, SIZEOF_STR(ip_addr_text5));
			web_buff[SIZEOF_STR(ip_addr_text5)] = 0;
			/*
			if(IoTpAd.ComCfg.Use_DHCP)
			{
				web_body_add_input_str(web_buff, 
					ip_text_auto, 
					local_text_checked);
			}*/
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 8:
			step = 9;
			memcpy(web_buff, local_text8, SIZEOF_STR(local_text8));
			web_buff[SIZEOF_STR(local_text8)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;	
		case 9:
			step = 10;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}


const char server_text1[]  = "<tr>\r\n\
<td width=\"33%\" height=\"43\"><div align=\"right\"><span class=\"STYLE16\">TCP端口号:</span></div></td>\r\n\
<td width=\"67%\"><div align=\"left\"><span class=\"STYLE16\">\r\n\
<input name=\"tcp_port\" type=\"text\" id=\"tcp_port\" maxlength=\"6\"/>\r\n\
</span></div></td>\r\n\
</tr>\r\n";

const char server_text2[]  = "<tr>\r\n\
<td width=\"33%\" height=\"43\"><div align=\"right\"><span class=\"STYLE16\">UDP端口号:</span></div></td>\r\n\
<td width=\"67%\"><div align=\"left\"><span class=\"STYLE16\">\r\n\
<input name=\"udp_port\" type=\"text\" id=\"udp_port\" maxlength=\"6\"/>\r\n\
</span></div></td>\r\n\
</tr>\r\n";

const char tcp_port_text[]  = "<input name=\"tcp_port\" type=\"text\" id=\"tcp_port\" maxlength=\"6\"/>";
const char udp_port_text[]  = "<input name=\"udp_port\" type=\"text\" id=\"udp_port\" maxlength=\"6\"/>";


int tcp_server_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	char tmp[64];
	//unsigned char ip_addr[4];
	int ret;
	if(_sub_step != -1)
		step = _sub_step;

	

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head));
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head));
			return ret;
		case 2:
			step = 3;
			make_sub_page_title("本地服务器配置");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 4;
			assert(SIZEOF_STR(server_text1) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, server_text1, SIZEOF_STR(server_text1));
			web_buff[SIZEOF_STR(server_text1)] = 0;
			sprintf(tmp, " value=\"%u\" ", web_cfg.server.port);
			web_body_add_input_str(web_buff, 
				tcp_port_text, 
				tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 4:
			step = 5;
			assert(SIZEOF_STR(server_text2) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, server_text2, SIZEOF_STR(server_text2));
			web_buff[SIZEOF_STR(server_text2)] = 0;
			sprintf(tmp, " value=\"%u\" ", web_cfg.server.port);
			web_body_add_input_str(web_buff, 
				udp_port_text, 
				tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 5:
			step = 6;
			memcpy(web_buff, local_text8, SIZEOF_STR(local_text8));
			web_buff[SIZEOF_STR(local_text8)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;	
		case 6:
			step = 7;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}

const char remote_text1[]  = "<tr>\r\n\
<td width=\"33%\" height=\"43\"><div align=\"right\"><span class=\"STYLE16\">端口号:</span></div></td>\r\n\
<td width=\"67%\"><div align=\"left\"><span class=\"STYLE16\">\r\n\
<input name=\"port\" type=\"text\" id=\"port\" maxlength=\"6\"/>\r\n\
</span></div></td>\r\n\
</tr>\r\n";

const char remote_port_text[]  = "<input name=\"port\" type=\"text\" id=\"port\" maxlength=\"6\"/>";


int remote_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	char tmp[64];
	//unsigned char ip_addr[4];
	int ret;
	if(_sub_step != -1)
		step = _sub_step;
	
	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head));
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head));
			return ret;
		case 2:
			step = 3;
			make_sub_page_title("远程控制配置");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 5;
			assert(SIZEOF_STR(remote_text1) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, remote_text1, SIZEOF_STR(remote_text1));
			web_buff[SIZEOF_STR(remote_text1)] = 0;
			sprintf(tmp, " value=\"%u\" ", /*IoTpAd.ComCfg.Remote_Ctl_Port*/4900);
			web_body_add_input_str(web_buff, 
				remote_port_text, 
				tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 5:
			step = 6;
			memcpy(web_buff, local_text8, SIZEOF_STR(local_text8));
			web_buff[SIZEOF_STR(local_text8)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;	
		case 6:
			step = 7;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}


const char uart_text1_1[]  = "<tr>\r\n\
    <td width=\"43%\" height=\"28\"><div align=\"right\"><span class=\"STYLE16\">波特率:</span></div></td>\r\n\
    <td width=\"57%\"><div align=\"left\"><span class=\"STYLE16\">\r\n\
      <select name=\"baud\" size=\"1\" id=\"baud\">\r\n";
     
const char uart_text1_2[]  = "<option value=\"9600\" >4800</option>\r\n\
<option value=\"9600\" >9600</option>\r\n\
<option value=\"19200\">19200</option>\r\n\
<option value=\"38400\">38400</option>\r\n\
<option value=\"57600\">57600</option>\r\n\
<option value=\"115200\">115200</option>\r\n\
<option value=\"230400\">230400</option>\r\n\
<option value=\"460800\">460800</option>\r\n\
<option value=\"921600\">921600</option>\r\n\
<option value=\"3200000\">3200000</option>\r\n\
</select>\r\n\
(bps)</span></div></td>\r\n\
</tr>\r\n";

const char uart_text2[]  = "<tr>\r\n\
<td height=\"32\"><div align=\"right\"><span class=\"STYLE16\">数据位:</span></div></td>\r\n\
<td><div align=\"left\"><span class=\"STYLE16\">\r\n\
<select name=\"data\" size=\"1\" id=\"data\">\r\n\
<option value=\"5\">5</option>\r\n\
<option value=\"6\">6</option>\r\n\
<option value=\"7\">7</option>\r\n\
<option value=\"8\">8</option>\r\n\
</select>\r\n\
(bit)</span></div></td>\r\n\
</tr>\r\n";

const char uart_text3[]  = "<td height=\"32\"><div align=\"right\"><span class=\"STYLE16\">校验:</span></div></td>\r\n\
<td><div align=\"left\"><span class=\"STYLE16\">\r\n\
<select name=\"parity\" size=\"1\" id=\"parity\">\r\n\
<option value=\"0\">NONE</option>\r\n\
<option value=\"1\">ODD</option>\r\n\
<option value=\"2\">EVEN</option>\r\n\
<option value=\"3\">SPACE</option>\r\n\
</select>\r\n\
</span></div></td>\r\n\
</tr>\r\n";

const char uart_text4[]  = "<tr>\r\n\
<td height=\"32\"><div align=\"right\"><span class=\"STYLE16\">停止位:</span></div></td>\r\n\
<td><div align=\"left\"><span class=\"STYLE16\">\r\n\
<select name=\"stopbits\" size=\"1\" id=\"stopbits\">\r\n\
<option value=\"1\">1</option>\r\n\
<option value=\"2\">2</option>\r\n\
<option value=\"3\">1.5</option>\r\n\
</select>\r\n\
</span></div></td>\r\n\
</tr>\r\n";

const char uart_text5[]  = "<tr>\r\n\
<td height=\"38\" colspan=\"2\"><div align=\"center\"><span class=\"STYLE16\">\r\n\
<input name=\"dtu_open\" type=\"checkbox\" id=\"dtu_open\" value=\"1\" />\r\n\
启用透传模式\r\n\
</span></div><span class=\"STYLE16\"><label></label>\r\n\
</span></td>\r\n\
</tr>\r\n\
<tr>\r\n";

const char dtu_text[]  = "<input name=\"dtu_open\" type=\"checkbox\" id=\"dtu_open\" value=\"1\" />";


int uart_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	char tmp[64];
	//unsigned char ip_addr[4];
	int ret;
	if(_sub_step != -1)
		step = _sub_step;

	

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head));
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head));
			return ret;
		case 2:
			step = 3;
			make_sub_page_title("UART配置");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 4;
			memcpy(web_buff, uart_text1_1, SIZEOF_STR(uart_text1_1));
			web_buff[SIZEOF_STR(uart_text1_1)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;		
		case 4:
			step = 5;
			assert(SIZEOF_STR(uart_text1_2) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, uart_text1_2, SIZEOF_STR(uart_text1_2));
			web_buff[SIZEOF_STR(uart_text1_2)] = 0;
			sprintf((char*)tmp, "option value=\"%d\"", web_cfg.dtu.baud);
			web_body_add_option_str(web_buff, tmp, " selected=\"selected\"");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 5:
			step = 6;
			assert(SIZEOF_STR(uart_text2) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, uart_text2, SIZEOF_STR(uart_text2));
			web_buff[SIZEOF_STR(uart_text2)] = 0;
			sprintf((char*)tmp, "option value=\"%d\"", /*IoTpAd.ComCfg.UART_DataBits*/8);
			web_body_add_option_str(web_buff, tmp, " selected=\"selected\"");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 6:
			step = 7;
			assert(SIZEOF_STR(uart_text3) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, uart_text3, SIZEOF_STR(uart_text3));
			web_buff[SIZEOF_STR(uart_text3)] = 0;
			sprintf((char*)tmp, "option value=\"%d\"", /*IoTpAd.ComCfg.UART_Parity*/0);
			web_body_add_option_str(web_buff, tmp, " selected=\"selected\"");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 7:
			step = 8;
			assert(SIZEOF_STR(uart_text4) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, uart_text4, SIZEOF_STR(uart_text4));
			web_buff[SIZEOF_STR(uart_text4)] = 0;
			sprintf((char*)tmp, "option value=\"%d\"", /*IoTpAd.ComCfg.UART_StopBits*/1);
			web_body_add_option_str(web_buff, tmp, " selected=\"selected\"");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 8:
			step = 9;
			assert(SIZEOF_STR(uart_text5) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, uart_text5, SIZEOF_STR(uart_text5));
			web_buff[SIZEOF_STR(uart_text5)] = 0;
			if(web_cfg.dtu.open)
			{
				web_body_add_input_str(web_buff, 
					dtu_text, 
					local_text_checked);
			}
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 9:
			step = 10;
			memcpy(web_buff, local_text8, SIZEOF_STR(local_text8));
			web_buff[SIZEOF_STR(local_text8)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;	
		case 10:
			step = 11;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}


const char reset_text1[]  = "<tr>\r\n\
<td height=\"42\"><div align=\"center\"><span class=\"STYLE16\">\r\n\
<input name=\"factory_reset\" type=\"checkbox\" id=\"factory_reset\" value=\"1\" />\r\n\
恢复出厂设置 </span></div>\r\n\
<span class=\"STYLE16\">\r\n\
<label></label>\r\n\
</span></td>\r\n\
</tr>\r\n";


int reset_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	//unsigned char ip_addr[4];
	int ret;
	if(_sub_step != -1)
		step = _sub_step;

	

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head));
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head));
			return ret;
		case 2:
			step = 3;
			make_sub_page_title("重启模块");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 5;
			//assert(SIZEOF_STR(reset_text1) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, reset_text1, SIZEOF_STR(reset_text1));
			web_buff[SIZEOF_STR(reset_text1)] = 0;
			//sprintf(tmp, " value=\"%u\" ", IoTpAd.ComCfg.Remote_Ctl_Port);
			//web_body_add_input_str(web_buff, 
			//	remote_port_text, 
			//	tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 5:
			step = 6;
			memcpy(web_buff, local_text8, SIZEOF_STR(local_text8));
			web_buff[SIZEOF_STR(local_text8)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;	
		case 6:
			step = 7;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}




const char update_text1[]  = "<tr><td><div align=\"center\">\r\n\
<input id=\"File1\" runat=\"server\" name=\"UpLoadFile\" type=\"file\" />\r\n\
</div></td></tr>\r\n";



const char update_text2[]  = "<tr>\r\n\
<td><div align=\"center\">\r\n\
<input type=\"submit\" name=\"Button1\" value=\"确定\"/>\r\n\
</div></td>\r\n\
</tr>\r\n";


int update_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	//unsigned char ip_addr[4];
	int ret;
	if(_sub_step != -1)
		step = _sub_step;

	

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head));
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head_update, SIZEOF_STR(http_body_head_update));
			return ret;
		case 2:
			step = 3;
			make_sub_page_title("固件升级");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 5;
			//assert(SIZEOF_STR(reset_text1) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, update_text1, SIZEOF_STR(reset_text1));
			web_buff[SIZEOF_STR(reset_text1)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 5:
			step = 6;
			memcpy(web_buff, update_text2, SIZEOF_STR(update_text2));
			web_buff[SIZEOF_STR(update_text2)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;	
		case 6:
			step = 7;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}


const char cust_text1[]  = "<tr>\r\n\
<td height=\"42\"><div align=\"center\"><span class=\"STYLE16\">\r\n\
<input name=\"set_index\" type=\"checkbox\" id=\"set_index\" value=\"1\" />\r\n\
设为主页</span></div>\r\n\
<span class=\"STYLE16\">\r\n\
<label></label>\r\n\
</span></td>\r\n\
</tr>\r\n";

const char cust_text2[]  = "<input name=\"set_index\" type=\"checkbox\" id=\"set_index\" value=\"1\" />";


int cust_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	//unsigned char ip_addr[4];
	int ret;
	if(_sub_step != -1)
		step = _sub_step;

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head));
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head));
			return ret;
		case 2:
			step = 3;
			make_sub_page_title("页面定制");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 5;
			memcpy(web_buff, cust_text1, SIZEOF_STR(cust_text1));
			web_buff[SIZEOF_STR(cust_text1)] = 0;
			/*
			if(IoTpAd.ComCfg.IOMode)
			{
				web_body_add_input_str(web_buff, 
					cust_text2, 
					local_text_checked);
			}*/
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 5:
			step = 6;
			memcpy(web_buff, local_text8, SIZEOF_STR(local_text8));
			web_buff[SIZEOF_STR(local_text8)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;	
		case 6:
			step = 7;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}


const char login_text1[]  = "<tr>\r\n\
<td width=\"31%\" height=\"45\"><div align=\"right\"><span class=\"STYLE16\">密码: </span></div>\r\n\
<span class=\"STYLE16\">\r\n\
<label></label>\r\n\
</span>\r\n\
<div align=\"right\"></div></td>\r\n\
<td width=\"69%\"><div align=\"left\"><span class=\"STYLE16\">\r\n\
<input name=\"log_password\" type=\"text\" id=\"link_key\" maxlength=\"8\"/>\r\n\
</span></div></td>\r\n\
</tr>\r\n";

const char login_text2[]  = "<input name=\"log_password\" type=\"text\" id=\"link_key\" maxlength=\"8\"/>";

int login_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	char tmp[64];
	//unsigned char ip_addr[4];
	int ret;
	if(_sub_step != -1)
		step = _sub_step;

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head));
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head));
			return ret;
		case 2:
			step = 3;
			make_sub_page_title("登陆密码配置");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 5;
			assert(SIZEOF_STR(login_text1) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, login_text1, SIZEOF_STR(login_text1));
			web_buff[SIZEOF_STR(login_text1)] = 0;
			//dump_hex("", &IoTpAd.ComCfg, sizeof(IoTpAd.ComCfg));
			sprintf(tmp, " value=\"%s\" ", web_cfg.password);
			//p_dbg("CmdPWD:%s", IoTpAd.ComCfg.CmdPWD);
			web_body_add_input_str(web_buff, 
				login_text2, 
				tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 5:
			step = 6;
			memcpy(web_buff, local_text8, SIZEOF_STR(local_text8));
			web_buff[SIZEOF_STR(local_text8)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;	
		case 6:
			step = 7;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}



const char io_text1[]  = "<tr>\r\n\
<td height=\"22\"><div align=\"center\"><span class=\"STYLE16\">IO1:</span>\r\n\
<input name=\"IO1\" type=\"checkbox\" id=\"IO1\" value=\"1\" />\r\n\
</div></td>\r\n\
</tr>\r\n";

const char io_text2[]  = "<input name=\"IO1\" type=\"checkbox\" id=\"IO1\" value=\"1\" />";

int io_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	//unsigned char ip_addr[4];
	uint8_t value = 0;
	int ret;
	if(_sub_step != -1)
		step = _sub_step;

	

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head));
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head));
			return ret;
		case 2:
			step = 3;
			make_sub_page_title("IO控制");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 4;
			memcpy(web_buff, io_text1, SIZEOF_STR(io_text1));
			web_buff[SIZEOF_STR(io_text1)] = 0;
			
			//IoT_gpio_read(0, &value, &polarity);
			if(value)
			{
				web_body_add_input_str(web_buff, 
					io_text2, 
					local_text_checked);
			}
			//str_replace(web_buff, "IO 1", "IO 1");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 4:
			step = 5;
			memcpy(web_buff, io_text1, SIZEOF_STR(io_text1));
			web_buff[SIZEOF_STR(io_text1)] = 0;
			
			//IoT_gpio_read(1, &value, &polarity);
			if(value)
			{
				web_body_add_input_str(web_buff, 
					io_text2, 
					local_text_checked);
			}
			str_replace(web_buff, "IO1", "IO2");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 5:
			step = 6;
			memcpy(web_buff, io_text1, SIZEOF_STR(io_text1));
			web_buff[SIZEOF_STR(io_text1)] = 0;
			
			//IoT_gpio_read(2, &value, &polarity);
			if(value)
			{
				web_body_add_input_str(web_buff, 
					io_text2, 
					local_text_checked);
			}
			str_replace(web_buff, "IO1", "IO3");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 6:
			step = 7;
			memcpy(web_buff, io_text1, SIZEOF_STR(io_text1));
			web_buff[SIZEOF_STR(io_text1)] = 0;
			
			//IoT_gpio_read(3, &value, &polarity);
			if(value)
			{
				web_body_add_input_str(web_buff, 
					io_text2, 
					local_text_checked);
			}
			str_replace(web_buff, "IO1", "IO4");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 7:
			step = 8;
			memcpy(web_buff, io_text1, SIZEOF_STR(io_text1));
			web_buff[SIZEOF_STR(io_text1)] = 0;
			
			//IoT_gpio_read(4, &value, &polarity);
			if(value)
			{
				web_body_add_input_str(web_buff, 
					io_text2, 
					local_text_checked);
			}
			str_replace(web_buff, "IO1", "IO5");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 8:
			step = 9;
			memcpy(web_buff, io_text1, SIZEOF_STR(io_text1));
			web_buff[SIZEOF_STR(io_text1)] = 0;
			
			//IoT_gpio_read(5, &value, &polarity);
			if(value)
			{
				web_body_add_input_str(web_buff, 
					io_text2, 
					local_text_checked);
			}
			str_replace(web_buff, "IO1", "IO6");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 9:
			step = 10;
			memcpy(web_buff, local_text8, SIZEOF_STR(local_text8));
			web_buff[SIZEOF_STR(local_text8)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;	
		case 10:
			step = 11;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}

/*
const char status_ok_page[] = "HTTP/1.1 200 OK\r\n\
Server:WSUM-WebServer-v1.0\r\n\
Connection: close\r\n\
Content-Length: 86\r\n\
Content-Type: text/html\r\n\
\r\n\
<html><head><title>ok</title></head><body><p align=\"center\">ok</p></p></body></html>";
*/

void send_post_status(int s, int _step)
{
	static uint8_t step = 0;
	if(_step != -1)
		step = _step;
	
	web_cfg.cur_routing = (void (*)(UCHAR, CHAR))send_post_status;
	p_dbg("status step:%d", step);
	
	if(step == 0)
	{
		sprintf(web_buff, "%s%d\r\n\r\n", http_text_head, sizeof(post_ok_page));
		web_data_send(s, web_buff, strlen(web_buff));
		step = 1;
	}else if(step == 1){
		web_data_send(s, (const char*)post_ok_page, sizeof(post_ok_page));	
		web_cfg.cur_routing = 0;
		step = 0;
	}
	web_cfg.need_reboot = 1;
}

const char client_text1[]  = "<tr>\r\n\
<td width=\"33%\" height=\"43\"><div align=\"right\"><span class=\"STYLE16\">服务器地址:</span></div></td>\r\n\
<td width=\"67%\"><span class=\"STYLE16\">\r\n\
<div align=\"left\">\r\n\
<input name=\"server_addr\" type=\"text\" id=\"server_addr\" maxlength=\"64\"/>\r\n\
</div>\r\n\
</span></td>\r\n\
</tr>\r\n";

const char server_addr_text[]  = "<input name=\"server_addr\" type=\"text\" id=\"server_addr\" maxlength=\"64\"/>";

int tcp_client_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	char tmp[64 + 1];
	//unsigned char ip_addr[4];
	int ret;
	if(_sub_step != -1)
		step = _sub_step;

	

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head));
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head));
			return ret;
		case 2:
			step = 3;
			make_sub_page_title("客户端配置");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 4;
			assert(SIZEOF_STR(client_text1) < WEB_BUFF_MAX_VALID);
			memcpy(web_buff, client_text1, SIZEOF_STR(client_text1));
			web_buff[SIZEOF_STR(client_text1)] = 0;
			/*
			if(!IoTpAd.ComCfg.IoT_ServeIP[0] && IoTpAd.ComCfg.IoT_ServeDomain[0])
			{
				IoTpAd.ComCfg.IoT_ServeDomain[64] = 0;
				sprintf(tmp, " value=\"%s\" ", IoTpAd.ComCfg.IoT_ServeDomain);
			}else{
				sprintf(tmp, " value=\"%u.%u.%u.%u\" ", IoTpAd.ComCfg.IoT_ServeIP[0], 
					IoTpAd.ComCfg.IoT_ServeIP[1], 
					IoTpAd.ComCfg.IoT_ServeIP[2], 
					IoTpAd.ComCfg.IoT_ServeIP[3]);
			}*/
			sprintf(tmp, " value=\"%u.%u.%u.%u\" ", ip4_addr1(&web_cfg.dtu.server_ip), 
					ip4_addr2(&web_cfg.dtu.server_ip), 
					ip4_addr3(&web_cfg.dtu.server_ip),  
					ip4_addr4(&web_cfg.dtu.server_ip));
					
			web_body_add_input_str(web_buff, 
				server_addr_text, 
				tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 4:
			step = 6/*5*/; //只使用tcp连接，这里直接跳到6
			memcpy(web_buff, server_text1, SIZEOF_STR(server_text1));
			web_buff[SIZEOF_STR(server_text1)] = 0;
			sprintf(tmp, " value=\"%u\" ", web_cfg.dtu.port);
			web_body_add_input_str(web_buff, 
				tcp_port_text, 
				tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 5:
			step = 6;
			memcpy(web_buff, server_text2, SIZEOF_STR(server_text2));
			web_buff[SIZEOF_STR(server_text2)] = 0;
			sprintf(tmp, " value=\"%u\" ", web_cfg.dtu.port);
			web_body_add_input_str(web_buff, 
				udp_port_text, 
				tmp);
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;	
		case 6:
			step = 7;
			memcpy(web_buff, local_text8, SIZEOF_STR(local_text8));
			web_buff[SIZEOF_STR(local_text8)] = 0;
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;	
		case 7:
			step = 8;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}

extern timer_t *led4_timer, *led5_timer;

int handle_wifilink_post(int s, char *buff)
{
	int i;
	char ssid[32 + 1];
	char key[32 + 1];
//	int auth_mode;
	//char pmk[FLASH_STA_CFG_4WAY_PMK_LEN]	
	p_dbg_enter;

	i = get_post_tag_value("link_mode", buff, 0);
	web_cfg.link.mode = (i == 1)?1:0;

	i = get_post_tag_value("link_channel", buff, 0);
	if(i > 0 && i < 14)
		i = i;
	else
		i = 1;
	web_cfg.link.channel = i;

	i = get_post_tag_value("link_ssid", buff, ssid);
	if(i == 0)
	{
		if(strlen(ssid) < 32)
			strcpy(web_cfg.link.essid, ssid);
	}
	i = get_post_tag_value("link_key", buff, key);
	if(i == 0)
	{
		if(strlen(key) < 32)
			strcpy(web_cfg.link.key, key);
	}else{
		memset(web_cfg.link.key, 0, 14);
	}
	
	i = get_post_tag_value("link_security", buff, 0);
//	auth_mode = i;
	
	send_post_status(s, 0);

	if(web_cfg.link.mode == 0)	//sta
	{

		p_dbg("连接到:%s, 密码:%s", web_cfg.link.essid, web_cfg.link.key);
		add_timer(led4_timer);		
		wifi_set_mode(MODE_STATION);
		
		wifi_connect(web_cfg.link.essid, web_cfg.link.key);
		
		if (is_wifi_connected())
		{
			int i, wait_time = 10;

			auto_get_ip();

			for (i = 0; i < wait_time; i++)
			{
				p_dbg("%d", i);
				if (p_netif->ip_addr.addr)
					break;
				sleep(1000);
			}

			if (p_netif->ip_addr.addr)
			{
				del_timer(led4_timer);	
				show_tcpip_info();
				send_cmd_to_self('y');
			}
		}
	}

	return 0;
	
}


int handle_ip_addr_post(int s, char *buff)
{
	int i;
	char tmp[64];
	p_dbg_enter;

	i = get_post_tag_value("ip_ipaddr", buff, tmp);
	if(i == 0){
		web_cfg.ip.ip = inet_addr(tmp);
	}

	i = get_post_tag_value("ip_gw", buff, tmp);
	if(i == 0){
		web_cfg.ip.gw = inet_addr(tmp);
	}

	i = get_post_tag_value("ip_mask", buff, tmp);
	if(i == 0){
		web_cfg.ip.msk = inet_addr(tmp);
	}

	i = get_post_tag_value("ip_dns", buff, tmp);
	if(i == 0){
		web_cfg.ip.dns = inet_addr(tmp);
	}

	i = get_post_tag_value("ip_auto", buff, 0);
	if(i != 1)
		web_cfg.ip.auto_get = 0;
	else
		web_cfg.ip.auto_get = 1;
	
	send_post_status(s, 0);

	//todo
	return 0;
	
}

int handle_tcp_server_post(int s, char *buff)
{
	int i;
	p_dbg_enter;

	i = get_post_tag_value("tcp_port", buff, 0);
	if(i < 0)
		web_cfg.server.port = 0;
	else
		web_cfg.server.port = i;

	i = get_post_tag_value("udp_port", buff, 0);
	if(i >= 0){
		//miss
	}
	
	send_post_status(s, 0);

	//todo
	return 0;
	
}

int handle_uart_post(int s, char *buff)
{
	int i;
	p_dbg_enter;

	i = get_post_tag_value("baud", buff, 0);
	if(i < 0)
		web_cfg.dtu.baud = 115200;
	else
		web_cfg.dtu.baud = i;

	i = get_post_tag_value("databits", buff, 0);
	if(i >= 0){
		//IoTpAd.ComCfg.UART_DataBits= i;
	}

	
	i = get_post_tag_value("parity", buff, 0);
	if(i >= 0){
		//IoTpAd.ComCfg.UART_Parity= i;
	}


	i = get_post_tag_value("stopbits", buff, 0);
	if(i >= 0){
		//IoTpAd.ComCfg.UART_StopBits= i;
	}


	i = get_post_tag_value("dtu_open", buff, 0);
	if(i != 1)
		web_cfg.dtu.open = 0;
	else
		web_cfg.dtu.open = 1;

	send_post_status(s, 0);

	//todo
	return 0;
	
}



int handle_remote_ctl_post(int s, char *buff)
{
	int i;
	p_dbg_enter;

	i = get_post_tag_value("port", buff, 0);
	if(i >= 0){
		//IoTpAd.ComCfg.Remote_Ctl_Port = i;
	}
	
	send_post_status(s, 0);

	//todo
	return 0;
	
}

int handle_login_post(int s, char *buff)
{
	int i;
	char tmp[64];
	p_dbg_enter;

	i = get_post_tag_value("log_password", buff, tmp);
	if(i == 0)
	{
		if(strlen(tmp) <= 16)
		{
			strcpy(web_cfg.password, tmp);
		}
	}else{
		memset(web_cfg.password, 0, 32);
	}
	
	send_post_status(s, 0);

	//todo
	return 0;
	
}

int handle_reset_post(int s, char *buff)
{
	int i;
	p_dbg_enter;

	i = get_post_tag_value("factory_reset", buff, 0);
	if(i == 1){
		p_dbg("reset");
		//reset_cfg();
	}
	send_post_status(s, 0);

	soft_reset();

	return 0;
	
}

int handle_cust_post(int s, char *buff)
{
	int i;
	i = get_post_tag_value("set_index", buff, 0);
	//if(i == 1){
	//	IoTpAd.ComCfg.IOMode = 1;
	//}else
	//	IoTpAd.ComCfg.IOMode = 0;
	i = i;
	
	//p_dbg("io mode:%d", IoTpAd.ComCfg.IOMode);
	send_post_status(s, 0);
	return 0;
	
}

int handle_update_post(int s, char *buff)
{
	p_dbg_enter;
	send_post_status(s, 0);
	return 0;
}

int handle_io_post(int s, char *buff)
{
	int i, ret;
	char tmp[8];
	p_dbg_enter;

	for(i = 0; i < 6; i++)
	{
		sprintf(tmp, "IO%d", i + 1);
		ret = get_post_tag_value(tmp, buff, 0);
		//if(ret == 1){
		//	IoT_gpio_output(i, 1);
		//}else
		//	IoT_gpio_output(i, 0);
		ret = ret;
	}
	return 0;
}




int handle_tcp_client_post(int s, char *buff)
{
	char tmp[64 + 1];
	int i;
	p_dbg_enter;

	i = get_post_tag_value("server_addr", buff, tmp);
	if(i == 0)
	{
		if(tmp[0] >= '0' && tmp[0] <= '9')
		{
			memset(web_cfg.dtu.server_name, 0, 65);
			web_cfg.dtu.server_ip = inet_addr(tmp);
		}
		else
		{
			web_cfg.dtu.server_ip = 0;
			strcpy(web_cfg.dtu.server_name, tmp);
		}
	}else{
		memset(web_cfg.dtu.server_name, 0, 65);
		web_cfg.dtu.server_ip = 0;
	}

	i = get_post_tag_value("tcp_port", buff, 0);
	if(i < 0)
		web_cfg.dtu.port = 0;
	else
		web_cfg.dtu.port = i;

	i = get_post_tag_value("udp_port", buff, 0);
	//if(i >= 0){
	//	IoTpAd.ComCfg.IoT_UDP_Srv_Port = i;
	//}

	send_post_status(s, 0);
	return 0;
	
}


/*
 *发送一个具体的页面，页面内容可先在Dreamweaver编辑，
 *然后放在这里分段发送出去，
 *上面assemble相关的函数使用相同的处理方式
 *
*/
int devcie_info_page_assemble(UCHAR s, CHAR _sub_step)
{
	static uint8_t step = 0;
	int ret;
	if(_sub_step != -1)
		step = _sub_step;

	p_dbg("inf ass:%d", step);

	switch(step)
	{
		case 0:
			step = 1;
			ret = web_data_send(s, page_head, SIZEOF_STR(page_head) - 1);
			return ret;
		case 1:
			step = 2;
			ret = web_data_send(s, http_body_head, SIZEOF_STR(http_body_head) - 1);
			return ret;
		case 2:
			step = 3;
			sprintf(web_buff, "<tr bordercolor=\"#F0F0F0\">\r\n\
    			<td height=\"94\" colspan=\"2\"><div align=\"center\"><span class=\"STYLE18\"><p>欢迎使用广州市勋睿科技发展有限公司的产品</p></span></div>\r\n\
				<p>电话:13025182598</p>\r\n\
				<p>QQ:381935393</p>\r\n\
				<p>更多产品请关注:wsum205.taobao.com</p>\r\n\
      			<hr /></td>\r\n\
  				</tr>\r\n");
  			ret = web_data_send(s, web_buff, strlen(web_buff));
			return ret;
		case 3:
			step = 4;
			ret = web_data_send(s, http_body_tail, SIZEOF_STR(http_body_tail));
			return ret;
		default:
			step = 0;
			return 0;
	}
}

/*
 *发送页面的公用函数，一个页面分为多次发送
 *具体页面通过assemble_routing进行组装和发送
 *
*/
void sub_page_send(UCHAR s, CHAR _step)
{
	static uint8_t step = 0;
	int ret, len = 0;
	if(_step != -1)
		step = _step;
	if(!web_cfg.assemble_routing)
		return;
	
	web_cfg.cur_routing = sub_page_send;
	p_dbg("sub step:%d", step);

	
	if(step == 0)
	{ //第一步socket赋值0xff，不能发送数据出去，只用于统计页面大小
		ret = web_cfg.assemble_routing(0xff, 0);
		while(ret)
		{
			len += ret;
			ret = web_cfg.assemble_routing(0xff, -1);
		}
		p_dbg("len:%d", len);

		sprintf(web_buff, "%s%d\r\n\r\n", http_text_head, len);
		web_data_send(s, web_buff, strlen(web_buff)); //获得页面长度后生成并发送http头
		
		step = 1;
	}else if(step == 1){
	//第二步socket赋实际值，将http的数据部分发送出去
	
		ret = web_cfg.assemble_routing(s, 0);
		step = 2;
		
		if(ret == 0){
			web_cfg.cur_routing = 0;
			web_cfg.web_sm = WEB_SM_IDLE;
			step = 0;
		}
	}else if(step == 2){
		ret = web_cfg.assemble_routing(s, -1);

		if(ret == 0){
			web_cfg.cur_routing = 0;
			web_cfg.web_sm = WEB_SM_IDLE;
			step = 0;
		}
	}
}

/*
 *发送主页，主页不需要修改参数，这里直接发生出去
 *其他页面则需要进行页面的组装，参考sub_page_send
 *
*/
void index_page_send(UCHAR s, CHAR _step)
{
	static uint8_t step = 0;
	if(_step != -1)
		step = _step;
	
	web_cfg.cur_routing = index_page_send;
	p_dbg("index step:%d", step);
	
	if(step == 0)
	{
		sprintf(web_buff, "%s%d\r\n\r\n", http_text_head, sizeof(index_page_data));
		web_data_send(s, web_buff, strlen(web_buff));
		step = 1;
		p_dbg("1");
	}else if(step == 1){
		web_data_send(s, (const char*)index_page_data, sizeof(index_page_data));	
		web_cfg.cur_routing = 0;
		step = 0;
	}
}

/*
 *发送logo图片
 *
 *
*/

void logo_gif_send(UCHAR s, CHAR _step)
{
	static uint8_t step = 0;
	if(_step != -1)
		step = _step;
	
	web_cfg.cur_routing = logo_gif_send;
	p_dbg("logo step:%d", step);
	
	if(step == 0)
	{
		sprintf(web_buff, "%s%d\r\n\r\n", http_gif_head, sizeof(logo_gif));
		web_data_send(s, web_buff, strlen(web_buff));
		step = 1;
	}else if(step == 1){
		web_data_send(s, (const char*)logo_gif, sizeof(logo_gif));	
		web_cfg.cur_routing = 0;
		step = 0;
	}
}

/*
 *处理get请求
 *
 *
*/
void handle_web_get(UCHAR s)
{
	if(web_cfg.get_name[0] == 0 || strcmp(web_cfg.get_name, str_index_page) == 0){
		index_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_device_info_page) == 0){
		web_cfg.assemble_routing = devcie_info_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_wifi_link_page) == 0){
		web_cfg.assemble_routing = wifi_link_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_ip_addr_page) == 0){
		web_cfg.assemble_routing = ip_addr_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_tcp_server_page) == 0){
		web_cfg.assemble_routing = tcp_server_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_tcp_client_page) == 0){
		web_cfg.assemble_routing = tcp_client_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_remote_ctl_page) == 0){
		web_cfg.assemble_routing = remote_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_reset_page) == 0){
		web_cfg.assemble_routing = reset_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_uart_page) == 0){
		web_cfg.assemble_routing = uart_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_io_page) == 0){
		web_cfg.assemble_routing = io_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_update_page) == 0){
		web_cfg.assemble_routing = update_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_cust_page) == 0){
		web_cfg.assemble_routing = cust_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_login_page) == 0){
		web_cfg.assemble_routing = login_page_assemble;
		sub_page_send(s, 0);
	}else if(strcmp(web_cfg.get_name, str_logo_gif) == 0)
		logo_gif_send(s, 0);
	else{
		p_err("unkown page:%s", web_cfg.get_name);
		web_data_send(s, err_404_page, SIZEOF_STR(err_404_page));
	}
}

/*
 *处理post请求
 *
 *
*/

void handle_web_post(int s)
{
	if(strcmp(web_cfg.get_name, str_wifi_link_page) == 0){
		handle_wifilink_post(s, web_buff);
	}else if(strcmp(web_cfg.get_name, str_ip_addr_page) == 0){
		handle_ip_addr_post(s, web_buff);
	}else if(strcmp(web_cfg.get_name, str_tcp_server_page) == 0){
		handle_tcp_server_post(s, web_buff);
	}else if(strcmp(web_cfg.get_name, str_tcp_client_page) == 0){
		handle_tcp_client_post(s, web_buff);
	}else if(strcmp(web_cfg.get_name, str_remote_ctl_page) == 0){
		handle_remote_ctl_post(s, web_buff);
	}else if(strcmp(web_cfg.get_name, str_reset_page) == 0){
		handle_reset_post(s, web_buff);
	}else if(strcmp(web_cfg.get_name, str_uart_page) == 0){
		handle_uart_post(s, web_buff);
	}else if(strcmp(web_cfg.get_name, str_io_page) == 0){
		handle_io_post(s, web_buff);
	}else if(strcmp(web_cfg.get_name, str_update_page) == 0){
		handle_update_post(s, web_buff);
	}else if(strcmp(web_cfg.get_name, str_cust_page) == 0){
		handle_cust_post(s, web_buff);
	}else if(strcmp(web_cfg.get_name, str_login_page) == 0){
		handle_login_post(s, web_buff);
	}else
		p_err("unkown page:%s", web_cfg.get_name);
}

int is_str_end(char c)
{
	if(c == '\r' || c == '\n')
		return 1;
	else
		return 0;
}

void web_update_timer(void)
{
	if(web_cfg.is_update_post && (web_cfg.web_sm == WEB_SM_POST_GETMORE1))
	{
		if(web_cfg.update_data_timer++ > 16)
		{
			p_err("update timeout");
			web_cfg.is_update_post = 0;
			web_cfg.update_data_cnt = 0;
			web_cfg.web_sm =  WEB_SM_IDLE;
			//erase_update_erea(UPDATE_DATA_BASE_ADDR, );
		}
	}
}

void web_abort(void)
{
	p_dbg_enter;
	web_cfg.web_sm = WEB_SM_IDLE;
	//if(web_cfg.need_reboot)
	//	Sys_reboot();
}

int handle_web_sm(UCHAR s, PUCHAR packet ,uint32_t packetlength)
{
	//p_dbg("page:%d", SIZEOF_STR(err_404_page));
	//dump_hex("w",err_404_page, SIZEOF_STR());
	//uip_send(err_404_page, SIZEOF_STR(err_404_page));
	
	int i;
	char *p_str;
#ifdef DEBUG
	//if(packet){
		//packet[packetlength] = 0;
		//p_dbg("pkg:%s", packet);
	//}
#endif
again:
	switch(web_cfg.web_sm)
	{
		case WEB_SM_IDLE: //idle状态下接受客户端的get和post请求
			if(strstr((const char*)packet, str_get)){
				p_dbg(str_get);
				web_cfg.web_sm = WEB_SM_GET_GETMORE;
				p_str = get_name((char*)str_get, (char*)packet, &i);
				if(p_str && i < GET_NAME_LEN)
				{
					memcpy(web_cfg.get_name, p_str, i);
					web_cfg.get_name[i] = 0;
				}else
					web_cfg.get_name[0] = 0;
			}else if(strstr((const char*)packet, str_post)){
				p_dbg(str_post);
				web_cfg.web_sm = WEB_SM_POST_GETMORE;
				web_cfg.post_len = 0;
				web_cfg.is_update_post = 0;
				p_str = get_name((char*)str_post, (char*)packet, &i);
				if(p_str && i < GET_NAME_LEN)
				{
					memcpy(web_cfg.get_name, p_str, i);
					web_cfg.get_name[i] = 0;
					if(strcmp((char*)str_update_page, web_cfg.get_name) == 0)
					{
						p_dbg("is update");
						/*
						web_cfg.is_update_post = 1;
						web_cfg.update_data_cnt = 0;
						web_cfg.update_data_timer = 0;
						web_cfg.update_addr = UPDATE_DATA_BASE_ADDR;
						web_cfg.update_remain_size = 0;
						erase_update_erea(UPDATE_DATA_BASE_ADDR, UPDATE_DATE_EREA_SIZE);*/
						p_err_miss;
						p_dbg("erase done");
					}					
				}else
					web_cfg.get_name[0] = 0;

				p_str = get_head_tag(str_content_len, (const char*)packet, &i);
				if(p_str && i < 8)
				{
					char tmp[8];
					memcpy(tmp, p_str, 8);
					tmp[i] = 0;
					web_cfg.post_len = atoi(tmp);
					p_dbg("post len:%d", web_cfg.post_len);
					
				}
			}

			p_dbg(web_cfg.get_name);
			
			if(web_cfg.web_sm > WEB_SM_IDLE){
				goto again;
			}
			break;
		//如果get请求数据没接收完，进入此模式(get请求只有head部分)
		case WEB_SM_GET_GETMORE: 
			if(strstr((const char*)packet, str_sect_end))
			{
				web_cfg.web_sm += 1;
				goto again;
			} else if(is_str_end(packet[0] ) && web_cfg.str_end_len) {
			 	int len = 1;
			 	if(is_str_end(packet[1] ))
					len++;
				if(is_str_end(packet[2] ))
					len++;
				if(is_str_end(packet[3] ))
					p_err("?");

				if((len + web_cfg.str_end_len) >= 4){
					web_cfg.web_sm += 1;
					p_dbg("end by half");
					goto again;
				}
			 }else if(is_str_end(packet[packetlength - 1])){
				web_cfg.str_end_len = 1;
				if(is_str_end(packet[packetlength - 2]))
					web_cfg.str_end_len ++;
				if(is_str_end(packet[packetlength - 3]))
					web_cfg.str_end_len ++;
				p_dbg("str half end:%d", web_cfg.str_end_len);
			}else
				web_cfg.str_end_len = 0;
	
			break;
		//如果post请求数据没接收完，进入此模式。
		//上传大数据(比如升级固件)也是通过post方式。
		case WEB_SM_POST_GETMORE: 
		{
			int remain;
			if(web_cfg.post_len == 0){ //
				p_str = get_head_tag(str_content_len, (const char*)packet, &i);
				if(p_str && i < 8)
				{
					char tmp[8];
					memcpy(tmp, p_str, 8);
					tmp[i] = 0;
					web_cfg.post_len = atoi(tmp);
					p_dbg("post len1:%d", web_cfg.post_len);
				}
			}

			p_str = strstr((const char*)packet, str_sect_end);
			if(p_str)
			{
				p_dbg("0:%s", p_str);
				p_str += SIZEOF_STR(str_sect_end);
				p_dbg("1:%d, %s", SIZEOF_STR(str_sect_end), p_str);
				goto find_end;
			}else if(is_str_end(packet[0] ) && web_cfg.str_end_len) {
			 	int len = 1;
			 	if(is_str_end(packet[1] ))
					len++;
				if(is_str_end(packet[2] ))
					len++;
				if(is_str_end(packet[3] ))
					p_err("?");

				if((len + web_cfg.str_end_len) >= 4){
					
					p_dbg("end by half");
					p_dbg("0:%s", packet);
					p_str = (char*)packet + len;
					p_dbg("1:%s", p_str);
					goto find_end;
				}
			 }else if(is_str_end(packet[packetlength - 1])){
				web_cfg.str_end_len = 1;
				if(is_str_end(packet[packetlength - 2]))
					web_cfg.str_end_len ++;
				if(is_str_end(packet[packetlength - 3]))
					web_cfg.str_end_len ++;
				p_dbg("str half end:%d", web_cfg.str_end_len);
			}else
				web_cfg.str_end_len = 0;
			break;
find_end:
			if(web_cfg.post_len == 0)
				web_cfg.web_sm = WEB_SM_RESPONE_POST;
			else{
				web_cfg.web_sm = WEB_SM_POST_GETMORE1;
				
				remain = packetlength + (uint32_t)packet - (uint32_t)p_str;
				if(((uint32_t)(packetlength + packet) > (uint32_t)p_str) && ((remain) < WEB_BUFF_MAX_VALID))
				{	
					if(web_cfg.is_update_post)
					{
						if(write_update_data((uint8_t*)p_str, remain) == -1)
						{
							p_err("write_update_data err");
							web_cfg.is_update_post = 0;
							web_cfg.update_data_cnt = 0;
							web_cfg.web_sm =  WEB_SM_IDLE;
						}
						web_cfg.update_data_cnt += remain; 
					}else{
						memcpy(web_buff, p_str, remain);
						web_buff[remain] = 0;
					}
					p_dbg("post remain:%d", remain);
				}else{
					p_err("plen:%d, remanlen:%d", packetlength, remain);
					memset(web_buff, 0, WEB_BUFF_SIZE);
				}

				packetlength = 0; //数据已经读到
			}
			p_dbg("post more next:%d", web_cfg.web_sm);
			goto again;
//			break;
		}
		//这里主要接收post请求的数据部分
		case WEB_SM_POST_GETMORE1:
			if(web_cfg.is_update_post) //update data
			{
				write_update_data(packet, packetlength);
				web_cfg.update_data_cnt += packetlength; 
				web_cfg.update_data_timer = 0;
				if(web_cfg.update_data_cnt >= web_cfg.post_len)
				{
					write_update_data_finish();
					web_cfg.web_sm += 1;
					goto again;
				}
				
			}else{
				i = strlen(web_buff);
				packetlength = ((packetlength + i) < WEB_BUFF_MAX_VALID)?packetlength:(WEB_BUFF_MAX_VALID - i);
				memcpy(web_buff, packet, packetlength);
				if(i + packetlength >= web_cfg.post_len) 
				{
					p_dbg("post data end:%s", web_buff);
					web_cfg.web_sm += 1;
					goto again;
				}
			}
			break;
		//应答get请求，(发送页面)
		case WEB_SM_RESPONE_GET:
			p_dbg("RESPONE_GET:%s", web_cfg.get_name);
			web_cfg.cur_routing = 0;
			web_cfg.web_sm = WEB_SM_IDLE; //web_sm在handle_web_get根据情况进行改变
			handle_web_get(s);
			if(web_cfg.web_sm == WEB_SM_SEND_DATA)
				goto again;
			break;
		//处理post请求，主要处理post过来的数据
		case WEB_SM_RESPONE_POST:
			p_dbg("RESPONE_POST");
			web_cfg.cur_routing = 0;
			web_cfg.web_sm = WEB_SM_IDLE;//同上， 必须放在handle_web_post前面
			handle_web_post(s);
			if(web_cfg.web_sm == WEB_SM_SEND_DATA)
				goto again;
			break;
		//发送数据状态(发送页面)
		case WEB_SM_SEND_DATA:
			while(web_cfg.cur_routing)
			{
				web_cfg.cur_routing(s, -1);
			}

			p_dbg("send web data end");
			web_cfg.web_sm = WEB_SM_IDLE;
			//if(web_cfg.need_reboot)
				//Sys_reboot();
			break;
		default:
			web_cfg.web_sm = WEB_SM_IDLE;
			break;
	}

	
	return 0;
}



void web_server_thread(void *arg)
{
	int socket_s = (int)arg;
	int ret, size;
	struct sockaddr addr;
	
	while(1)
	{
		size = sizeof(struct sockaddr);
		ret = accept(socket_s, &addr, (socklen_t*)&size);
		if(ret == -1)
		{
			p_err("accept err1");
			sleep(1000);
		}			
	}
}

void web_server_init(void)
{
	int  socket_s, err;
	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(struct sockaddr_in));
	
	memset(&web_cfg, 0, sizeof(WEB_CFG));
	web_cfg.web_sm = WEB_SM_IDLE;

	serv.sin_family = AF_INET;
	serv.sin_port = htons(80);	
	serv.sin_addr.s_addr = htons(INADDR_ANY);

	mutex_lock(socket_mutex);		
	socket_s = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_s == -1)
	{
		p_err("%s err1", __FUNCTION__);
		mutex_unlock(socket_mutex);
		return;
	}
	err = bind(socket_s, (struct sockaddr*)&serv, sizeof(struct sockaddr_in));
	if(err == -1)
	{
		p_err("%s err2", __FUNCTION__);
		mutex_unlock(socket_mutex);
		return;
	}
	err = listen(socket_s, 4);
	if(err == -1)
	{
		p_err("%s err3", __FUNCTION__);
		mutex_unlock(socket_mutex);
		return;
	}
	mutex_unlock(socket_mutex);	

	thread_create(web_server_thread, (void*)socket_s, TASK_WEB_PRIO, 0, TASK_WEB_STK_SIZE, "tcp_recv_thread");
}

#endif

