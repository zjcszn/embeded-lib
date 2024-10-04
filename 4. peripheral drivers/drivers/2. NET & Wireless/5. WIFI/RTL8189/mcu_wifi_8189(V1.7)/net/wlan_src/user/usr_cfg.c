#define DEBUG
#include "debug.h"
#include "lwip/sockets.h"
#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <rtw_byteorder.h>

#include <hal_intf.h>
#include "ctype.h"
#include "wpa.h"
void do_gettimeofday(struct timeval *t)
{
	t->tv_sec = os_time_get()/1000;
	t->tv_usec = os_time_get()%1000;
}

union ktime timeval_to_ktime(struct timeval *t)
{
	union ktime kt;
	kt.tv64 = t->tv_sec*1000 + t->tv_usec;
	return kt;
}

void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}


void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}


/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

void __list_del_entry(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = (struct list_head *)entry;
	entry->prev = (struct list_head *)entry;
}

int copy_from_user(void *dst, void *src, int size)
{
	memcpy(dst, src, size);
	return 0;
}

int	try_module_get(int arg)
{
	arg = 1;
	return arg;
}
/** Put module */
int	module_put(int arg)
{
	arg= 1;
	return arg;
}

struct ieee80211_channel *ieee80211_get_channel(struct wiphy *wiphy,
						  int freq)
{
	enum ieee80211_band band;
	struct ieee80211_supported_band *sband;
	int i;

	for (band = (enum ieee80211_band)0; band < IEEE80211_NUM_BANDS; band++) {
		sband = wiphy->bands[band];

		if (!sband)
			continue;

		for (i = 0; i < sband->n_channels; i++) {
			if (sband->channels[i].center_freq == freq)
				return &sband->channels[i];
		}
	}

	return NULL;
}

#if 0
#define RSN_SELECTOR_PUT(a, val) WPA_PUT_BE32((u8 *) (a), (val))
#define RSN_SELECTOR_GET(a) WPA_GET_BE32((const u8 *) (a))

static int wpa_selector_to_bitfield(const u8 *s)
{
	if (RSN_SELECTOR_GET(s) == RSN_SELECTOR_GET(WPA_CIPHER_SUITE_NONE))
		return WPA_CIPHER_NONE;
	if (RSN_SELECTOR_GET(s) == RSN_SELECTOR_GET(WPA_CIPHER_SUITE_WEP40))
		return WPA_CIPHER_WEP40;
	if (RSN_SELECTOR_GET(s) == RSN_SELECTOR_GET(WPA_CIPHER_SUITE_TKIP))
		return WPA_CIPHER_TKIP;
	if (RSN_SELECTOR_GET(s) == RSN_SELECTOR_GET(WPA_CIPHER_SUITE_CCMP))
		return WPA_CIPHER_CCMP;
	if (RSN_SELECTOR_GET(s) == RSN_SELECTOR_GET(WPA_CIPHER_SUITE_WEP104))
		return WPA_CIPHER_WEP104;
	return 0;
}


static int wpa_key_mgmt_to_bitfield(const u8 *s)
{
	if (RSN_SELECTOR_GET(s) == RSN_SELECTOR_GET(WPA_AUTH_KEY_MGMT_UNSPEC_802_1X))
		return WPA_KEY_MGMT_IEEE8021X;
	if (RSN_SELECTOR_GET(s) == RSN_SELECTOR_GET(WPA_AUTH_KEY_MGMT_PSK_OVER_802_1X))
		return WPA_KEY_MGMT_PSK;
	if (RSN_SELECTOR_GET(s) == RSN_SELECTOR_GET(WPA_AUTH_KEY_MGMT_NONE))
		return WPA_KEY_MGMT_WPA_NONE;
	return 0;
}


static int rsn_selector_to_bitfield(const u8 *s)
{
	if (RSN_SELECTOR_GET(s) == RSN_SELECTOR_GET(RSN_CIPHER_SUITE_NONE))
		return WPA_CIPHER_NONE;
	if (RSN_SELECTOR_GET(s) == RSN_SELECTOR_GET(RSN_CIPHER_SUITE_WEP40))
		return WPA_CIPHER_WEP40;
	if (RSN_SELECTOR_GET(s) == RSN_SELECTOR_GET(RSN_CIPHER_SUITE_TKIP))
		return WPA_CIPHER_TKIP;
		if (RSN_SELECTOR_GET(s) == RSN_SELECTOR_GET(RSN_CIPHER_SUITE_CCMP))
		return WPA_CIPHER_CCMP;
	if (RSN_SELECTOR_GET(s) == RSN_SELECTOR_GET(RSN_CIPHER_SUITE_WEP104))
		return WPA_CIPHER_WEP104;
#ifdef CONFIG_IEEE80211W
	if (RSN_SELECTOR_GET(s) == RSN_CIPHER_SUITE_AES_128_CMAC)
		return WPA_CIPHER_AES_128_CMAC;
#endif /* CONFIG_IEEE80211W */
	return 0;
}


static int rsn_key_mgmt_to_bitfield(const u8 *s)
{
	if (RSN_SELECTOR_GET(s) == RSN_SELECTOR_GET(RSN_AUTH_KEY_MGMT_UNSPEC_802_1X))
		return WPA_KEY_MGMT_IEEE8021X;
	if (RSN_SELECTOR_GET(s) == RSN_SELECTOR_GET(RSN_AUTH_KEY_MGMT_PSK_OVER_802_1X))
		return WPA_KEY_MGMT_PSK;
#ifdef CONFIG_IEEE80211R
	if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_FT_802_1X)
		return WPA_KEY_MGMT_FT_IEEE8021X;
	if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_FT_PSK)
		return WPA_KEY_MGMT_FT_PSK;
#endif /* CONFIG_IEEE80211R */
#ifdef CONFIG_IEEE80211W
	if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_802_1X_SHA256)
		return WPA_KEY_MGMT_IEEE8021X_SHA256;
	if (RSN_SELECTOR_GET(s) == RSN_AUTH_KEY_MGMT_PSK_SHA256)
		return WPA_KEY_MGMT_PSK_SHA256;
#endif /* CONFIG_IEEE80211W */
	return 0;
}


int wpa_parse_wpa_ie_rsn(const u8 *rsn_ie, size_t rsn_ie_len,
			 struct wpa_ie_data *data)
{
#ifndef CONFIG_NO_WPA2
	const struct rsn_ie_hdr *hdr;
	const u8 *pos;
	int left;
	int i, count;

	memset(data, 0, sizeof(*data));
	data->proto = WPA_PROTO_RSN;
	data->pairwise_cipher = WPA_CIPHER_CCMP;
	data->group_cipher = WPA_CIPHER_CCMP;
	data->key_mgmt = WPA_KEY_MGMT_IEEE8021X;
	data->capabilities = 0;
	data->pmkid = NULL;
	data->num_pmkid = 0;
#ifdef CONFIG_IEEE80211W
	data->mgmt_group_cipher = WPA_CIPHER_AES_128_CMAC;
#else /* CONFIG_IEEE80211W */
	data->mgmt_group_cipher = 0;
#endif /* CONFIG_IEEE80211W */

	if (rsn_ie_len == 0) {
		/* No RSN IE - fail silently */
		return -1;
	}

	if (rsn_ie_len < sizeof(struct rsn_ie_hdr)) {
		wpa_printf(MSG_DEBUG, "%s: ie len too short %lu",
			   "", (unsigned long) rsn_ie_len);
		return -1;
	}

	hdr = (const struct rsn_ie_hdr *) rsn_ie;

	if (hdr->elem_id != WLAN_EID_RSN ||
	    hdr->len != rsn_ie_len - 2 ||
	    WPA_GET_LE16(hdr->version) != RSN_VERSION) {
		wpa_printf(MSG_DEBUG, "%s: malformed ie or unknown version",
			   "");
		return -2;
	}

	pos = (const u8 *) (hdr + 1);
	left = rsn_ie_len - sizeof(*hdr);

	if (left >= RSN_SELECTOR_LEN) {
		data->group_cipher = rsn_selector_to_bitfield(pos);
#ifdef CONFIG_IEEE80211W
		if (data->group_cipher == WPA_CIPHER_AES_128_CMAC) {
			wpa_printf(MSG_DEBUG, "%s: AES-128-CMAC used as group "
				   "cipher", "");
			return -1;
		}
#endif /* CONFIG_IEEE80211W */
		pos += RSN_SELECTOR_LEN;
		left -= RSN_SELECTOR_LEN;
	} else if (left > 0) {
		wpa_printf(MSG_DEBUG, "%s: ie length mismatch, %u too much",
			   "", left);
		return -3;
	}

	if (left >= 2) {
		data->pairwise_cipher = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * RSN_SELECTOR_LEN) {
			wpa_printf(MSG_DEBUG, "%s: ie count botch (pairwise), "
				   "count %u left %u", "", count, left);
			return -4;
		}
		for (i = 0; i < count; i++) {
			data->pairwise_cipher |= rsn_selector_to_bitfield(pos);
			pos += RSN_SELECTOR_LEN;
			left -= RSN_SELECTOR_LEN;
		}
#ifdef CONFIG_IEEE80211W
		if (data->pairwise_cipher & WPA_CIPHER_AES_128_CMAC) {
			wpa_printf(MSG_DEBUG, "%s: AES-128-CMAC used as "
				   "pairwise cipher", "");
			return -1;
		}
#endif /* CONFIG_IEEE80211W */
	} else if (left == 1) {
		wpa_printf(MSG_DEBUG, "%s: ie too short (for key mgmt)",
			   "");
		return -5;
	}

	if (left >= 2) {
		data->key_mgmt = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * RSN_SELECTOR_LEN) {
			wpa_printf(MSG_DEBUG, "%s: ie count botch (key mgmt), "
				   "count %u left %u", "", count, left);
			return -6;
		}
		for (i = 0; i < count; i++) {
			data->key_mgmt |= rsn_key_mgmt_to_bitfield(pos);
			pos += RSN_SELECTOR_LEN;
			left -= RSN_SELECTOR_LEN;
		}
	} else if (left == 1) {
		wpa_printf(MSG_DEBUG, "%s: ie too short (for capabilities)",
			   "");
		return -7;
	}

	if (left >= 2) {
		data->capabilities = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
	}

	if (left >= 2) {
		data->num_pmkid = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (left < (int) data->num_pmkid * PMKID_LEN) {
			wpa_printf(MSG_DEBUG, "%s: PMKID underflow "
				   "(num_pmkid=%lu left=%d)",
				   "", (unsigned long) data->num_pmkid,
				   left);
			data->num_pmkid = 0;
			return -9;
		} else {
			data->pmkid = pos;
			pos += data->num_pmkid * PMKID_LEN;
			left -= data->num_pmkid * PMKID_LEN;
		}
	}

#ifdef CONFIG_IEEE80211W
	if (left >= 4) {
		data->mgmt_group_cipher = rsn_selector_to_bitfield(pos);
		if (data->mgmt_group_cipher != WPA_CIPHER_AES_128_CMAC) {
			wpa_printf(MSG_DEBUG, "%s: Unsupported management "
				   "group cipher 0x%x", "",
				   data->mgmt_group_cipher);
			return -10;
		}
		pos += RSN_SELECTOR_LEN;
		left -= RSN_SELECTOR_LEN;
	}
#endif /* CONFIG_IEEE80211W */

	if (left > 0) {
		wpa_printf(MSG_DEBUG, "%s: ie has %u trailing bytes - ignored",
			   "", left);
	}

	return 0;
#else /* CONFIG_NO_WPA2 */
	return -1;
#endif /* CONFIG_NO_WPA2 */
}

static int wpa_parse_wpa_ie_wpa(const u8 *wpa_ie, size_t wpa_ie_len,
				struct wpa_ie_data *data)
{
	const struct wpa_ie_hdr *hdr;
	const u8 *pos;
	int left;
	int i, count;

	memset(data, 0, sizeof(*data));
	data->proto = WPA_PROTO_WPA;
	data->pairwise_cipher = WPA_CIPHER_TKIP;
	data->group_cipher = WPA_CIPHER_TKIP;
	data->key_mgmt = WPA_KEY_MGMT_IEEE8021X;
	data->capabilities = 0;
	data->pmkid = NULL;
	data->num_pmkid = 0;
	data->mgmt_group_cipher = 0;

	if (wpa_ie_len == 0) {
		/* No WPA IE - fail silently */
		return -1;
	}

	if (wpa_ie_len < sizeof(struct wpa_ie_hdr)) {
		wpa_printf(MSG_DEBUG, "%s: ie len too short %lu",
			   "", (unsigned long) wpa_ie_len);
		return -1;
	}

	hdr = (const struct wpa_ie_hdr *) wpa_ie;

	if (hdr->elem_id != WLAN_EID_VENDOR_SPECIFIC ||
	    hdr->len != wpa_ie_len - 2 ||
	    RSN_SELECTOR_GET(hdr->oui) != RSN_SELECTOR(0x00, 0x50, 0xf2, 1) ||
	    WPA_GET_LE16(hdr->version) != WPA_VERSION) {
		wpa_printf(MSG_DEBUG, "%s: malformed ie or unknown version",
			   "");
		return -1;
	}

	pos = (const u8 *) (hdr + 1);
	left = wpa_ie_len - sizeof(*hdr);

	if (left >= WPA_SELECTOR_LEN) {
		data->group_cipher = wpa_selector_to_bitfield(pos);
		pos += WPA_SELECTOR_LEN;
		left -= WPA_SELECTOR_LEN;
	} else if (left > 0) {
		wpa_printf(MSG_DEBUG, "%s: ie length mismatch, %u too much",
			   "", left);
		return -1;
	}

	if (left >= 2) {
		data->pairwise_cipher = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * WPA_SELECTOR_LEN) {
			wpa_printf(MSG_DEBUG, "%s: ie count botch (pairwise), "
				   "count %u left %u", "", count, left);
			return -1;
		}
		for (i = 0; i < count; i++) {
			data->pairwise_cipher |= wpa_selector_to_bitfield(pos);
			pos += WPA_SELECTOR_LEN;
			left -= WPA_SELECTOR_LEN;
		}
	} else if (left == 1) {
		wpa_printf(MSG_DEBUG, "%s: ie too short (for key mgmt)",
			   "");
		return -1;
	}

	if (left >= 2) {
		data->key_mgmt = 0;
		count = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
		if (count == 0 || left < count * WPA_SELECTOR_LEN) {
			wpa_printf(MSG_DEBUG, "%s: ie count botch (key mgmt), "
				   "count %u left %u", "", count, left);
			return -1;
		}
		for (i = 0; i < count; i++) {
			data->key_mgmt |= wpa_key_mgmt_to_bitfield(pos);
			pos += WPA_SELECTOR_LEN;
			left -= WPA_SELECTOR_LEN;
		}
	} else if (left == 1) {
		wpa_printf(MSG_DEBUG, "%s: ie too short (for capabilities)",
			   "");
		return -1;
	}

	if (left >= 2) {
		data->capabilities = WPA_GET_LE16(pos);
		pos += 2;
		left -= 2;
	}

	if (left > 0) {
		wpa_printf(MSG_DEBUG, "%s: ie has %u trailing bytes - ignored",
			   "", left);
	}

	return 0;
}


int wpa_parse_wpa_ie(const u8 *wpa_ie, size_t wpa_ie_len,
		     struct wpa_ie_data *data)
{
	if (wpa_ie_len >= 1 && wpa_ie[0] == WLAN_EID_RSN)
		return wpa_parse_wpa_ie_rsn(wpa_ie, wpa_ie_len, data);
	else
		return wpa_parse_wpa_ie_wpa(wpa_ie, wpa_ie_len, data);
}
#endif
int wpa_parse_wpa_ie(const u8 *wpa_ie, size_t wpa_ie_len,
		     struct wpa_ie_data *data);

int wpa_sm_parse_own_wpa_ie(struct wpa_sm *sm, struct wpa_ie_data *data)
{
	if (sm == NULL || sm->assoc_wpa_ie == NULL) {
		wpa_printf(MSG_DEBUG, "WPA: No WPA/RSN IE available from "
			   "association info");
		return -1;
	}
	if (wpa_parse_wpa_ie(sm->assoc_wpa_ie, sm->assoc_wpa_ie_len, data))
		return -2;
	return 0;
}


void (*usr_scan_result_callback)(BSS_TABLE_SIMPLE *Tab) = NULL;
struct wlan_network *lastest_scan_result; //保存最近一次的扫描结果

void handle_user_scan_result(struct wlan_network *pnetwork)
{
	//struct scan_result_data result;
//	struct wpa_ie_data ie_data;
	
	lastest_scan_result = pnetwork;
}

struct cfg80211_bss*
cfg80211_inform_bss(struct wiphy *wiphy,
		    struct ieee80211_channel *channel,
		    const u8 *bssid,
		    const u8 *essid,
		    u8 essid_len,
		    u64 timestamp, u16 capability, u16 beacon_interval,
		    const u8 *ie, size_t ielen,
		    s32 signal, gfp_t gfp)
{
	struct cfg80211_internal_bss *res;
	size_t privsz;

	if (!wiphy)
		return NULL;

	privsz = /*wiphy->bss_priv_size*/0;
	
	if ((wiphy->signal_type == CFG80211_SIGNAL_TYPE_UNSPEC &&
			(signal < 0 || signal > 100))){
		return NULL;
	}
	res = kzalloc(sizeof(*res) + privsz + ielen, gfp);
	if (!res)
		return NULL;

	memcpy(res->pub.bssid, bssid, ETH_ALEN);
	res->pub.channel = channel;
	res->pub.signal = signal;
	res->pub.tsf = timestamp;
	res->pub.beacon_interval = beacon_interval;
	res->pub.capability = capability;
	/*
	 * Since we do not know here whether the IEs are from a Beacon or Probe
	 * Response frame, we need to pick one of the options and only use it
	 * with the driver that does not provide the full Beacon/Probe Response
	 * frame. Use Beacon frame pointer to avoid indicating that this should
	 * override the information_elements pointer should we have received an
	 * earlier indication of Probe Response data.
	 *
	 * The initial buffer for the IEs is allocated with the BSS entry and
	 * is located after the private area.
	 */
	res->pub.beacon_ies = (u8 *)res + sizeof(*res) + privsz;
	memcpy(res->pub.beacon_ies, ie, ielen);
	res->pub.len_beacon_ies = ielen;
	res->pub.information_elements = res->pub.beacon_ies;
	res->pub.len_information_elements = res->pub.len_beacon_ies;

	memcpy(res->pub.ssid, essid, 32);
	res->pub.ssid_len = essid_len;
	
	//kref_init(&res->ref);

	//res = cfg80211_bss_update(wiphy_to_dev(wiphy), res);
	//if (!res)
	//	return NULL;

	//if (res->pub.capability & WLAN_CAPABILITY_ESS)
	//	regulatory_hint_found_beacon(wiphy, channel, gfp);

	/* cfg80211_bss_update gives us a referenced result */
	return &res->pub;
}

void wiphy_free(struct wiphy *wiphy)
{
	p_err_miss;
}

void wiphy_unregister(struct wiphy *wiphy)
{
	p_err_miss;
}


int ieee80211_channel_to_frequency(int chan, enum ieee80211_band band)
{
	/* see 802.11 17.3.8.3.2 and Annex J
	 * there are overlapping channel numbers in 5GHz and 2GHz bands */
	if (band == IEEE80211_BAND_5GHZ) {
		if (chan >= 182 && chan <= 196)
			return 4000 + chan * 5;
		else
			return 5000 + chan * 5;
	} else { /* IEEE80211_BAND_2GHZ */
		if (chan == 14)
			return 2484;
		else if (chan < 14)
			return 2407 + chan * 5;
		else
			return 0; /* not supported */
	}
}

int ieee80211_frequency_to_channel(int freq)
{
	/* see 802.11 17.3.8.3.2 and Annex J */
	if (freq == 2484)
		return 14;
	else if (freq < 2484)
		return (freq - 2407) / 5;
	else if (freq >= 4910 && freq <= 4980)
		return (freq - 4000) / 5;
	else
		return (freq - 5000) / 5;
}

int strnicmp(const char *s1, const char *s2, size_t len)
{
	/* Yes, Virginia, it had better be unsigned */
	unsigned char c1, c2;

	if (!len)
		return 0;

	do {
		c1 = *s1++;
		c2 = *s2++;
		if (!c1 || !c2)
			break;
		if (c1 == c2)
			continue;
		c1 = tolower(c1);
		c2 = tolower(c2);
		if (c1 != c2)
			break;
	} while (--len);
	return (int)c1 - (int)c2;
}


void cfg80211_scan_done(struct rtw_wdev_priv *pwdev_priv, bool aborted)
{
	_adapter *pad;
	pad = pwdev_priv->padapter;		
	
	if(pwdev_priv->scan_request){
		mem_free(pwdev_priv->scan_request);
	}

	if(usr_scan_result_callback)
		usr_scan_result_callback(&pad->mlmepriv.ScanTabSimple);
	
	usr_scan_result_callback = NULL;

	//在这里可用添加用户自定义代码
	p_dbg("cfg80211_scan_done");
}

void cfg80211_connect_result(struct net_device *dev, const u8 *bssid,
			     const u8 *req_ie, size_t req_ie_len,
			     const u8 *resp_ie, size_t resp_ie_len,
			     u16 status, gfp_t gfp)
{
	//在这里可用添加用户自定义代码
	p_dbg("cfg80211_connect_result status:%d", status);
}

void cfg80211_cqm_rssi_notify(struct net_device *dev,
			      enum nl80211_cqm_rssi_threshold_event rssi_event,
			      gfp_t gfp)
{
	//在这里可用添加用户自定义代码
	p_err_miss;
}

void cfg80211_michael_mic_failure(struct net_device *dev, const u8 *addr,
				  enum nl80211_key_type key_type, int key_id,
				  const u8 *tsc, gfp_t gfp)
{
	//在这里可用添加用户自定义代码
	p_err_miss;
}


void cfg80211_ibss_joined(struct net_device *dev, const u8 *bssid, gfp_t gfp)
{
	//在这里可用添加用户自定义代码
	p_err_miss;
}


void cfg80211_put_bss(struct cfg80211_bss *pub)
{
	struct cfg80211_internal_bss *bss;

	if (!pub)
		return;

	bss = container_of(pub, struct cfg80211_internal_bss, pub);
	//kref_put(&bss->ref, bss_release);
	mem_free(bss);
}

void cfg80211_remain_on_channel_expired(struct wireless_dev *wdev, u64 cookie,
					struct ieee80211_channel *chan,
					gfp_t gfp)
{
	p_err_miss;
}

void cfg80211_ready_on_channel(struct wireless_dev *wdev, u64 cookie,
			       struct ieee80211_channel *chan,
			       unsigned int duration, gfp_t gfp)
{
	p_err_miss;
}

void wireless_send_event(struct net_device *	dev,
			 unsigned int		cmd,
			 union iwreq_data *	wrqu,
			 const char *		extra)
{
    p_err("wireless event:%d", cmd);
}


__be16 eth_type_trans(struct sk_buff *skb, struct net_device *dev)
{
	//struct ethhdr *eth;

	//skb->dev = dev;
	skb_reset_mac_header(skb);
	skb_pull_inline(skb, ETH_HLEN);
#if 0		//包类型由协议栈处理
	eth = eth_hdr(skb);

	if (unlikely(is_multicast_ether_addr(eth->h_dest)))
	{
		if (!compare_ether_addr_64bits(eth->h_dest, dev->broadcast))
			skb->pkt_type = PACKET_BROADCAST;
		else
			skb->pkt_type = PACKET_MULTICAST;
	} 

	/*
	 *      This ALLMULTI check should be redundant by 1.4
	 *      so don't forget to remove it.
	 *
	 *      Seems, you forgot to remove it. All silly devices
	 *      seems to set IFF_PROMISC.
	 */

	else if (1 /*dev->flags&IFF_PROMISC */)
	{
		if (unlikely(compare_ether_addr_64bits(eth->h_dest, dev->dev_addr)))
			skb->pkt_type = PACKET_OTHERHOST;
	}

	/*
	 * Some variants of DSA tagging don't have an ethertype field
	 * at all, so we check here whether one of those tagging
	 * variants has been configured on the receiving interface,
	 * and if so, set skb->protocol without looking at the packet.
	 */
	//	if (netdev_uses_dsa_tags(dev))
	//		return htons(ETH_P_DSA);
	//	if (netdev_uses_trailer_tags(dev))
	//		return htons(ETH_P_TRAILER);

	if (ntohs(eth->h_proto) >= 1536)
		return eth->h_proto;

	/*
	 *      This is a magic hack to spot IPX packets. Older Novell breaks
	 *      the protocol design and runs IPX over 802.3 without an 802.2 LLC
	 *      layer. We look for FFFF which isn't a used 802.2 SSAP/DSAP. This
	 *      won't work for fault tolerant netware but does for the rest.
	 */
	if (skb->len >= 2 && *(unsigned short*)(skb->data) == 0xFFFF)
		return htons(ETH_P_802_3);

	/*
	 *      Real 802.2 LLC
	 */

	return htons(ETH_P_802_2);
#endif
	return 0;
}


extern void ethernetif_input(struct netif *netif,void *p_buf,int size);

extern struct netif *p_netif;
int netif_rx(struct sk_buff *skb)
{
	if(p_netif)
	{
		ethernetif_input(p_netif, skb_mac_header(skb), skb->len + sizeof(struct ethhdr));
	}
	kfree_skb(skb);
	return  0;
} 


