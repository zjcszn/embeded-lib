/*
 *	Routines having to do with the 'struct sk_buff' memory handlers.
 *
 *	Authors:	Alan Cox <alan@lxorguk.ukuu.org.uk>
 *			Florian La Roche <rzsfl@rz.uni-sb.de>
 *
 *	Fixes:
 *		Alan Cox	:	Fixed the worst of the load
 *					balancer bugs.
 *		Dave Platt	:	Interrupt stacking fix.
 *	Richard Kooijman	:	Timestamp fixes.
 *		Alan Cox	:	Changed buffer format.
 *		Alan Cox	:	destructor hook for AF_UNIX etc.
 *		Linus Torvalds	:	Better skb_clone.
 *		Alan Cox	:	Added skb_copy.
 *		Alan Cox	:	Added all the changed routines Linus
 *					only put in the headers
 *		Ray VanTassle	:	Fixed --skb->lock in free
 *		Alan Cox	:	skb_copy copy arp field
 *		Andi Kleen	:	slabified it.
 *		Robert Olsson	:	Removed skb_head_pool
 *
 *	NOTE:
 *		The __skb_ routines should be called with interrupts
 *	disabled, or you better be *real* sure that the operation is atomic
 *	with respect to whatever list is being frobbed (e.g. via lock_sock()
 *	or via disabling bottom half handlers, etc).
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

/*
 *	The functions in this file will not compile correctly with gcc 2.4.x
 */
//#define DEBUG
#include "usr_cfg.h"
#include "debug.h"
#include "api.h"
#include "skbuff.h"
#include "netdevice.h"
#include "lwip/arch.h"

static uint32_t 	g_skb_alloc_size = 0;

void get_page(struct page *page)
{
	p_dbg_enter;
	if (page->addr)
		p_err("addr != 0\n");

	page->addr = (char*)mem_malloc(4096);
	if (!page->addr)
		p_err("no mem\n");
	g_skb_alloc_size += 4096;
} 

void put_page(struct page *page)
{
	p_dbg_enter;
	if (!page->addr)
	{
		p_err("addr == 0\n");
		return ;
	}
	mem_free(page->addr);
	page->addr = 0;
	g_skb_alloc_size -= 4096;
}

//static unsigned short from32to16(unsigned int x)
//{
//	/* add up 16-bit and 16-bit for 16+c bit */
//	x = (x &0xffff) + (x >> 16);
//	/* add up carry.. */
//	x = (x &0xffff) + (x >> 16);
//	return x;
//}
static void skb_dst_drop(struct sk_buff *skb)
{
	// 	p_err_miss;
	/*	if (skb->_skb_refdst) {
	refdst_drop(skb->_skb_refdst);
	skb->_skb_refdst = 0UL;
	}8/
	}

	static  void skb_dst_copy(struct sk_buff *nskb, const struct sk_buff *oskb)
	{
	p_err_miss;
	nskb->_skb_refdst = oskb->_skb_refdst;
	if (!(nskb->_skb_refdst & SKB_DST_NOREF))
	dst_clone(skb_dst(nskb));*/
} 


//应该用不到
//static void sg_set_buf(struct scatterlist *sg, const void *buf, unsigned int
//	buflen)
//	p_err_miss;
//	//g_set_page(sg, virt_to_page(buf), buflen, offset_in_page(buf));
//} 


/*
static int skb_pagelen(const struct sk_buff *skb)
{
	int i, len = 0;

	for (i = (int)skb_shinfo(skb)->nr_frags - 1; i >= 0; i--)
		len += skb_shinfo(skb)->frags[i].size;
	return len + skb_headlen(skb);
} */
/**
static void skb_fill_page_desc(struct sk_buff *skb, int i, struct page *page,
	int off, int size)
{
	skb_frag_t *frag = &skb_shinfo(skb)->frags[i];

	frag->page = page;
	frag->page_offset = off;
	frag->size = size;
	skb_shinfo(skb)->nr_frags = i + 1;
} */

//extern void skb_add_rx_frag(struct sk_buff *skb, int i, struct page *page, int
//	off, int size);

//#define SKB_PAGE_ASSERT(skb) 	BUG_ON(skb_shinfo(skb)->nr_frags)
//#define SKB_FRAG_ASSERT(skb) 	BUG_ON(skb_has_frag_list(skb))
#define SKB_LINEAR_ASSERT(skb)  BUG_ON(skb_is_nonlinear(skb))


/**
 *	skb_tailroom - bytes at buffer end
 *	@skb: buffer to check
 *
 *	Return the number of bytes of free space at the tail of an sk_buff
 */
int skb_tailroom(const struct sk_buff *skb)
{
	return skb_is_nonlinear(skb) ? 0 : skb->end - skb->tail;
} 

/**
 *	skb_reserve - adjust headroom
 *	@skb: buffer to alter
 *	@len: bytes to move
 *
 *	Increase the headroom of an empty &sk_buff by reducing the tail
 *	room. This is only allowed for an empty buffer.
 */
void skb_reserve(struct sk_buff *skb, int len)
{
	skb->data += len;
	skb->tail += len;
} 
unsigned char *skb_mac_header(const struct sk_buff *skb)
{
	return skb->mac_header;
} 

static int skb_mac_header_was_set(const struct sk_buff *skb)
{
	return skb->mac_header != NULL;
} 

void skb_reset_mac_header(struct sk_buff *skb)
{
	skb->mac_header = skb->data;
} 


/*
 * CPUs often take a performance hit when accessing unaligned memory
 * locations. The actual performance hit varies, it can be small if the
 * hardware handles it or large if we have to take an exception and fix it
 * in software.
 *
 * Since an ethernet header is 14 bytes network drivers often end up with
 * the IP header at an unaligned offset. The IP header can be aligned by
 * shifting the start of the packet by 2 bytes. Drivers should do this
 * with:
 *
 * skb_reserve(skb, NET_IP_ALIGN);
 *
 * The downside to this alignment of the IP header is that the DMA is now
 * unaligned. On some architectures the cost of an unaligned DMA is high
 * and this cost outweighs the gains made by aligning the IP header.
 *
 * Since this trade off varies between architectures, we allow NET_IP_ALIGN
 * to be overridden.
 */
#ifndef NET_IP_ALIGN
#define NET_IP_ALIGN	2
#endif 

/*
 * The networking layer reserves some headroom in skb data (via
 * dev_alloc_skb). This is used to avoid having to reallocate skb data when
 * the header has to grow. In the default case, if the header has to grow
 * 32 bytes or less we avoid the reallocation.
 *
 * Unfortunately this headroom changes the DMA alignment of the resulting
 * network packet. As for NET_IP_ALIGN, this unaligned DMA is expensive
 * on some architectures. An architecture can override this value,
 * perhaps setting it to a cacheline in size (since that will maintain
 * cacheline alignment of the DMA). It must be a power of 2.
 *
 * Various parts of the networking layer expect at least 32 bytes of
 * headroom, you should not reduce this.
 *
 * Using max(32, L1_CACHE_BYTES) makes sense (especially with RPS)
 * to reduce average number of cache lines per packet.
 * get_rps_cpus() for example only access one 64 bytes aligned block :
 * NET_IP_ALIGN(2) + ethernet_header(14) + IP_header(20/40) + ports(8)
 */
#ifndef NET_SKB_PAD
#define NET_SKB_PAD	max(32, 4)
#endif 

struct sk_buff *__dev_alloc_skb(unsigned int length, gfp_t gfp_mask)
{
	struct sk_buff *skb = alloc_skb(length + NET_SKB_PAD, gfp_mask);
	if (likely(skb))
		skb_reserve(skb, NET_SKB_PAD);
	
#ifdef DEBUG
	if(length  + NET_SKB_PAD == 112)
		p_dbg("alloc:%x, %x", skb, skb->head);
	//else
	p_dbg("alloc1:%d,%x, %x", length + NET_SKB_PAD, skb, skb->head);
#endif
	return skb;
} 

extern struct sk_buff *dev_alloc_skb(unsigned int length);

#define skb_queue_walk(queue, skb) \
for (skb = (queue)->next;					\
skb != (struct sk_buff *)(queue);				\
skb = skb->next)

#define skb_queue_walk_safe(queue, skb, tmp)					\
for (skb = (queue)->next, tmp = skb->next;			\
skb != (struct sk_buff *)(queue);				\
skb = tmp, tmp = skb->next)

#define skb_queue_walk_from(queue, skb)						\
for (; skb != (struct sk_buff *)(queue);			\
skb = skb->next)

#define skb_queue_walk_from_safe(queue, skb, tmp)				\
for (tmp = skb->next;						\
skb != (struct sk_buff *)(queue);				\
skb = tmp, tmp = skb->next)

#define skb_queue_reverse_walk(queue, skb) \
for (skb = (queue)->prev;					\
skb != (struct sk_buff *)(queue);				\
skb = skb->prev)

#define skb_queue_reverse_walk_safe(queue, skb, tmp)				\
for (skb = (queue)->prev, tmp = skb->prev;			\
skb != (struct sk_buff *)(queue);				\
skb = tmp, tmp = skb->prev)

#define skb_queue_reverse_walk_from_safe(queue, skb, tmp)			\
for (tmp = skb->prev;						\
skb != (struct sk_buff *)(queue);				\
skb = tmp, tmp = skb->prev)

int skb_copy_bits(const struct sk_buff *skb, int offset, void *to, int len);

void skb_copy_from_linear_data(const struct sk_buff *skb, void *to,
	const unsigned int len)
{
	memcpy(to, skb->data, len);
} 

void skb_copy_from_linear_data_offset(const struct sk_buff *skb, const
	int offset, void *to, const unsigned int len)
{
	memcpy(to, skb->data + offset, len);
} 


/**
 * skb_complete_tx_timestamp() - deliver cloned skb with tx timestamps
 *
 * @skb: clone of the the original outgoing packet
 * @hwtstamps: hardware time stamps
 *
 */
void skb_complete_tx_timestamp(struct sk_buff *skb, struct skb_shared_hwtstamps
	*hwtstamps);


/* Note: This doesn't put any conntrack and bridge info in dst. */
static void __nf_copy(struct sk_buff *dst, const struct sk_buff *src)
{
	#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
	dst->nfct = src->nfct;
	nf_conntrack_get(src->nfct);
	dst->nfctinfo = src->nfctinfo;
	#endif 
	#ifdef NET_SKBUFF_NF_DEFRAG_NEEDED
	dst->nfct_reasm = src->nfct_reasm;
	nf_conntrack_get_reasm(src->nfct_reasm);
	#endif 
	#ifdef CONFIG_BRIDGE_NETFILTER
	dst->nf_bridge = src->nf_bridge;
	nf_bridge_get(src->nf_bridge);
	#endif 
} 

static void skb_copy_secmark(struct sk_buff *to, const struct sk_buff *from){}


static void skb_copy_queue_mapping(struct sk_buff *to, const struct sk_buff
	*from)
{
	to->queue_mapping = from->queue_mapping;
} 

static void skb_over_panic(struct sk_buff *skb, int sz, void *here)
{
	p_err("skb_over_panic: text:%p len:%d put:%d head:%p "
	"data:%p tail:%#lx end:%#lx dev:%s\n", here, skb->len, sz, skb->head, skb
	->data, (unsigned long)skb->tail, (unsigned long)skb->end, skb->dev ? skb
	->dev->name: "<NULL>");
	BUG();
} 


/**
 *	skb_under_panic	- 	private function
 *	@skb: buffer
 *	@sz: size
 *	@here: address
 *
 *	Out of line support code for skb_push(). Not user callable.
 */

static void skb_under_panic(struct sk_buff *skb, int sz, void *here)
{
	p_err("skb_under_panic: text:%p len:%d put:%d head:%p "
	"data:%p tail:%#lx end:%#lx dev:%s\n", here, skb->len, sz, skb->head, skb
	->data, (unsigned long)skb->tail, (unsigned long)skb->end, skb->dev ? skb
	->dev->name: "<NULL>");
	BUG();
} 


/* 	Allocate a new skbuff. We do this ourselves so we can fill in a few
 *	'private' fields and also do memory statistics to find all the
 *	[BEEP] leaks.
 *
 */

/**
 *	__alloc_skb	-	allocate a network buffer
 *	@size: size to allocate
 *	@gfp_mask: allocation mask
 *	@fclone: allocate from fclone cache instead of head cache
 *		and allocate a cloned (child) skb
 *	@node: numa node to allocate memory on
 *
 *	Allocate a new &sk_buff. The returned buffer has no headroom and a
 *	tail room of size bytes. The object has a reference count of one.
 *	The return is the buffer. On a failure the return is %NULL.
 *
 *	Buffers may only be allocated from interrupts using a @gfp_mask of
 *	%GFP_ATOMIC.
 */
//DECLARE_MONITOR_ITEM("global skb alloc size", _g_skb_alloc_size);

struct sk_buff *__alloc_skb(const char* name, unsigned int size, gfp_t gfp_mask, int fclone, int
	node)
{
	uint32_t ret_size;
	//	struct kmem_cache *cache;
	struct skb_shared_info *shinfo;
	struct sk_buff *skb;
	u8 *data;
	
	skb = mem_calloc_ex(name, sizeof(struct sk_buff), 1);
	if (!skb)
		goto out;

	g_skb_alloc_size += get_mem_size(skb);
	
	size = SKB_DATA_ALIGN(size);
	data = mem_malloc(size + sizeof(struct skb_shared_info));

	if (!data)
		goto nodata;
	//	prefetchw(data + size);

	g_skb_alloc_size += get_mem_size(data);
	/*
	 * Only clear those fields we need to clear, not those that we will
	 * actually initialise below. Hence, don't put any more fields after
	 * the tail pointer in struct sk_buff!
	 */
	memset(skb, 0, offsetof(struct sk_buff, tail));
	skb->truesize = size + sizeof(struct sk_buff);
	atomic_set(&skb->users, 1);
	skb->head = data;
	skb->data = data;
	skb_reset_tail_pointer(skb);
	skb->end = skb->tail + size;
	#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb->mac_header = ~0U;
	#endif 
	//SET_MONITOR_ITEM_VALUE(_g_skb_alloc_size, g_skb_alloc_size);
	/* make sure we initialize shinfo sequentially */
	shinfo = skb_shinfo(skb);
	memset(shinfo, 0, /*offsetof(struct skb_shared_info, dataref)*/sizeof(struct skb_shared_info));
	atomic_set(&shinfo->dataref, 1);
	//	kmemcheck_annotate_variable(shinfo->destructor_arg);

	if (fclone)
	{
		p_err("fclone\n");
	} 
out: 
	return skb;
nodata: 
	ret_size = kmem_cache_free(cache, skb);

	g_skb_alloc_size -= ret_size;
	skb = NULL;
	goto out;
}


/**
 *	__netdev_alloc_skb - allocate an skbuff for rx on a specific device
 *	@dev: network device to receive on
 *	@length: length to allocate
 *	@gfp_mask: get_free_pages mask, passed to alloc_skb
 *
 *	Allocate a new &sk_buff and assign it a usage count of one. The
 *	buffer has unspecified headroom built in. Users should allocate
 *	the headroom they think they need without accounting for the
 *	built in space. The built in space is used for optimisations.
 *
 *	%NULL is returned if there is no free memory.
 */
struct sk_buff *__netdev_alloc_skb(struct net_device *dev, unsigned int length,
	gfp_t gfp_mask)
{
	struct sk_buff *skb;

	skb = __alloc_skb(__FUNCTION__, length + NET_SKB_PAD, gfp_mask, 0, NUMA_NO_NODE);
	if (likely(skb))
	{
		skb_reserve(skb, NET_SKB_PAD);
		skb->dev = dev;
	} return skb;
}

/**
 *	dev_alloc_skb - allocate an skbuff for receiving
 *	@length: length to allocate
 *
 *	Allocate a new &sk_buff and assign it a usage count of one. The
 *	buffer has unspecified headroom built in. Users should allocate
 *	the headroom they think they need without accounting for the
 *	built in space. The built in space is used for optimisations.
 *
 *	%NULL is returned if there is no free memory. Although this function
 *	allocates memory it can be called from an interrupt.
 */
struct sk_buff *dev_alloc_skb(unsigned int length)
{
	/*
	 * There is more code here than it seems:
	 * __dev_alloc_skb is an 
	 */
	return __dev_alloc_skb(length, GFP_ATOMIC);
} 



static void skb_release_data(struct sk_buff *skb)
{
	uint32_t ret_size;
	struct skb_shared_info *info = (struct skb_shared_info *)skb_end_pointer(skb);
	
	if (!skb->cloned || !atomic_sub_return(1,  &info->dataref))
	{
	/*
		if (info->nr_frags)
		{
			int i;
			for (i = 0; i < info->nr_frags; i++)
				put_page(info->frags[i].page);
		} 
	*/
		//if (skb_has_frag_list(skb))
		//	skb_drop_fraglist(skb);

		if(info->dataref.val && skb->cloned)
			p_err("dataref:%d", info->dataref.val);

		ret_size = kfree(skb->head);

		g_skb_alloc_size -= ret_size;
	}
}

/*
 *	Free an skbuff by memory without cleaning the state.
 */
static void kfree_skbmem(struct sk_buff *skb)
{
	uint32_t ret_size;
	struct sk_buff *other;
	atomic_t *fclone_ref;

	switch (skb->fclone)
	{
		case SKB_FCLONE_UNAVAILABLE:
			
			ret_size = kmem_cache_free(skbuff_head_cache, skb);

			g_skb_alloc_size -= ret_size;
			break;

		case SKB_FCLONE_ORIG:
			fclone_ref = (atomic_t*)(skb + 2);
			if (atomic_dec_and_test(fclone_ref)){
				ret_size = kmem_cache_free(skbuff_fclone_cache, skb);

				g_skb_alloc_size -= ret_size;
			}
			break;

		case SKB_FCLONE_CLONE:
			fclone_ref = (atomic_t*)(skb + 1);
			other = skb - 1;

			/* The clone portion is available for
			 * fast-cloning again.
			 */
			skb->fclone = SKB_FCLONE_UNAVAILABLE;

			if (atomic_dec_and_test(fclone_ref)){
				ret_size = kmem_cache_free(skbuff_fclone_cache, other);

				g_skb_alloc_size -= ret_size;
			}
			break;
	}
}

static void skb_release_head_state(struct sk_buff *skb)
{
	skb_dst_drop(skb);
	#ifdef CONFIG_XFRM
	secpath_put(skb->sp);
	#endif 
	if (skb->destructor)
	{
		//	WARN_ON(in_irq());
		skb->destructor(skb);
	} 
	#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
	nf_conntrack_put(skb->nfct);
	#endif 
	#ifdef NET_SKBUFF_NF_DEFRAG_NEEDED
	nf_conntrack_put_reasm(skb->nfct_reasm);
	#endif 
	#ifdef CONFIG_BRIDGE_NETFILTER
	nf_bridge_put(skb->nf_bridge);
	#endif 
	/* XXX: IS this still necessary? - JHS */
	#ifdef CONFIG_NET_SCHED
	skb->tc_index = 0;
	#ifdef CONFIG_NET_CLS_ACT
	skb->tc_verd = 0;
	#endif 
	#endif 
}

/* Free everything but the sk_buff shell. */
static void skb_release_all(struct sk_buff *skb)
{
	skb_release_head_state(skb);
	skb_release_data(skb);
} 

/**
 *	__kfree_skb - private function
 *	@skb: buffer
 *
 *	Free an sk_buff. Release anything attached to the buffer.
 *	Clean the state. This is an internal helper function. Users should
 *	always call kfree_skb
 */

void __kfree_skb(struct sk_buff *skb)
{
	skb_release_all(skb);
	kfree_skbmem(skb);
	//SET_MONITOR_ITEM_VALUE(_g_skb_alloc_size, g_skb_alloc_size);
} 


uint32_t get_global_skb_alloc()
{
	return g_skb_alloc_size;
}
/**
 *	kfree_skb - free an sk_buff
 *	@skb: buffer to free
 *
 *	Drop a reference to the buffer and free it if the usage count has
 *	hit zero.
 */
void kfree_skb(struct sk_buff *skb)
{
	if (unlikely(!skb))
		return ;

	else if (likely(!atomic_dec_and_test(&skb->users)))
		return ;

	__kfree_skb(skb);
} 

/**
 *	consume_skb - free an skbuff
 *	@skb: buffer to free
 *
 *	Drop a ref to the buffer and free it if the usage count has hit zero
 *	Functions identically to kfree_skb, but kfree_skb assumes that the frame
 *	is being dropped after a failure and notes that
 */
void consume_skb(struct sk_buff *skb)
{
	if (unlikely(!skb))
		return ;

	else if (likely(!atomic_dec_and_test(&skb->users)))
		return ;

	__kfree_skb(skb);
}

static void __copy_skb_header(struct sk_buff *new, const struct sk_buff *old)
{
	new->tstamp = old->tstamp;
	new->dev = old->dev;
	new->transport_header = old->transport_header;
	new->network_header = old->network_header;
	new->mac_header = old->mac_header;
	//skb_dst_copy(new, old);
	//p_err("skb_dst_copy\n");
	new->rxhash = old->rxhash;
	#ifdef CONFIG_XFRM
	new->sp = secpath_get(old->sp);
	#endif 
	memcpy(new->cb, old->cb, sizeof(old->cb));
	new->cs.csum = old->cs.csum;
	new->local_df = old->local_df;
	new->pkt_type = old->pkt_type;
	new->ip_summed = old->ip_summed;
	skb_copy_queue_mapping(new, old);
	new->priority = old->priority;
	#if defined(CONFIG_IP_VS) || defined(CONFIG_IP_VS_MODULE)
	new->ipvs_property = old->ipvs_property;
	#endif 
	new->protocol = old->protocol;
	new->mk.mark = old->mk.mark;
	new->skb_iif = old->skb_iif;
	__nf_copy(new, old);
	#if defined(CONFIG_NETFILTER_XT_TARGET_TRACE) || \
	defined(CONFIG_NETFILTER_XT_TARGET_TRACE_MODULE)
	new->nf_trace = old->nf_trace;
	#endif 
	#ifdef CONFIG_NET_SCHED
	new->tc_index = old->tc_index;
	#ifdef CONFIG_NET_CLS_ACT
	new->tc_verd = old->tc_verd;
	#endif 
	#endif 
	new->vlan_tci = old->vlan_tci;

	skb_copy_secmark(new, old);
} 

/*
 * You should not add any new code to this function.  Add it to
 * __copy_skb_header above instead.
 */
static struct sk_buff *__skb_clone(struct sk_buff *n, struct sk_buff *skb)
{
	#define C(x) n->x = skb->x

	n->next = n->prev = NULL;
	//n->sk = NULL;
	__copy_skb_header(n, skb);

	C(len);
	C(data_len);
	C(mac_len);
	n->hdr_len = skb->hdr_len;
	n->cloned = 1;
	//n->nohdr = 0;
	n->destructor = NULL;
	C(tail);
	C(end);
	C(head);
	C(data);
	C(truesize);
	atomic_set(&n->users, 1);

	atomic_inc(&(skb_shinfo(skb)->dataref));
	skb->cloned = 1;

	return n;
	#undef C
} 

/**
 *	skb_morph	-	morph one skb into another
 *	@dst: the skb to receive the contents
 *	@src: the skb to supply the contents
 *
 *	This is identical to skb_clone except that the target skb is
 *	supplied by the user.
 *
 *	The target skb is returned upon exit.
 */
struct sk_buff *skb_morph(struct sk_buff *dst, struct sk_buff *src)
{
	skb_release_all(dst);
	return __skb_clone(dst, src);
};

EXPORT_SYMBOL(skb_morph);

/**
 *	skb_clone	-	duplicate an sk_buff
 *	@skb: buffer to clone
 *	@gfp_mask: allocation priority
 *
 *	Duplicate an &sk_buff. The new one is not owned by a socket. Both
 *	copies share the same packet data but not structure. The new
 *	buffer has a reference count of 1. If the allocation fails the
 *	function returns %NULL otherwise the new buffer is returned.
 *
 *	If this function is called from an interrupt gfp_mask() must be
 *	%GFP_ATOMIC.
 */

struct sk_buff *skb_clone(struct sk_buff *skb, gfp_t gfp_mask)
{
	struct sk_buff *n;

	n = (struct sk_buff*)mem_calloc(1, sizeof(struct sk_buff));
	if (!n)
		return NULL;
	g_skb_alloc_size += get_mem_size(n);
	n->fclone = SKB_FCLONE_UNAVAILABLE;

	return __skb_clone(n, skb);
}

EXPORT_SYMBOL(skb_clone);

static void copy_skb_header(struct sk_buff *new, const struct sk_buff *old)
{
	#ifndef NET_SKBUFF_DATA_USES_OFFSET
	/*
	 *	Shift between the two data areas in bytes
	 */
	unsigned long offset = new->data - old->data;
	#endif 

	__copy_skb_header(new, old);

	#ifndef NET_SKBUFF_DATA_USES_OFFSET
	/* {transport,network,mac}_header are relative to skb->head */
	new->transport_header += offset;
	new->network_header += offset;
	if (skb_mac_header_was_set(new))
		new->mac_header += offset;
	#endif 
} 

/**
 *	skb_copy	-	create private copy of an sk_buff
 *	@skb: buffer to copy
 *	@gfp_mask: allocation priority
 *
 *	Make a copy of both an &sk_buff and its data. This is used when the
 *	caller wishes to modify the data and needs a private copy of the
 *	data to alter. Returns %NULL on failure or the pointer to the buffer
 *	on success. The returned buffer has a reference count of 1.
 *
 *	As by-product this function converts non-linear &sk_buff to linear
 *	one, so that &sk_buff becomes completely private and caller is allowed
 *	to modify all the data of returned buffer. This means that this
 *	function is not recommended for use in circumstances when only
 *	header is going to be modified. Use pskb_copy() instead.
 */

struct sk_buff *skb_copy(const struct sk_buff *skb, gfp_t gfp_mask)
{
	int headerlen = skb_headroom(skb);
	unsigned int size = (skb_end_pointer(skb) - skb->head) + skb->data_len;
	struct sk_buff *n = alloc_skb(size, gfp_mask);

	if (!n)
		return NULL;

	/* Set the data pointer */
	skb_reserve(n, headerlen);
	/* Set the tail pointer and length */
	skb_put(n, skb->len);

	if (skb_copy_bits(skb,  - headerlen, n->head, headerlen + skb->len))
		BUG();

	copy_skb_header(n, skb);
	return n;
} EXPORT_SYMBOL(skb_copy);


/**
 *	skb_put - add data to a buffer
 *	@skb: buffer to use
 *	@len: amount of data to add
 *
 *	This function extends the used data area of the buffer. If this would
 *	exceed the total buffer size the kernel will panic. A pointer to the
 *	first byte of the extra data is returned.
 */

unsigned char *skb_put(struct sk_buff *skb, unsigned int len)
{
	unsigned char *tmp = skb_tail_pointer(skb);
	//SKB_LINEAR_ASSERT(skb);
	skb->tail += len;
	skb->len += len;
	if (unlikely(skb->tail > skb->end))
		skb_over_panic(skb, len, 0 /*__builtin_return_address(0)*/);
	return tmp;
} EXPORT_SYMBOL(skb_put);

/**
 *	skb_push - add data to the start of a buffer
 *	@skb: buffer to use
 *	@len: amount of data to add
 *
 *	This function extends the used data area of the buffer at the buffer
 *	start. If this would exceed the total buffer headroom the kernel will
 *	panic. A pointer to the first byte of the extra data is returned.
 */
unsigned char *skb_push(struct sk_buff *skb, unsigned int len)
{
	skb->data -= len;
	skb->len += len;
	if (unlikely(skb->data < skb->head))
		skb_under_panic(skb, len, 0 /*__builtin_return_address(0)*/);
	return skb->data;
} EXPORT_SYMBOL(skb_push);

/* Copy some data bits from skb to kernel buffer. */

int skb_copy_bits(const struct sk_buff *skb, int offset, void *to, int len)
{
	int start = skb_headlen(skb);
	int copy;

	if (offset > (int)skb->len - len)
		goto fault;

	/* Copy header. */
	if ((copy = start - offset) > 0)
	{
		if (copy > len)
			copy = len;
		skb_copy_from_linear_data_offset(skb, offset, to, copy);
		if ((len -= copy) == 0)
			return 0;
		offset += copy;
		(char*)to += copy;
	} 
	if (!len)
		return 0;

fault: 
	p_err_fun;
	return  - EFAULT;
}

