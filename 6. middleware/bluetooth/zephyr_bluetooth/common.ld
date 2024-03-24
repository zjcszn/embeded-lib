        . = ALIGN(8);
        *(._k_queue.static.*)
        *(._k_sem.static.*)
        *(._k_mutex.static.*)
        _bt_gatt_service_static_list_start = .;
        KEEP(*(SORT_BY_NAME("._bt_gatt_service_static.static.*")))
        _bt_gatt_service_static_list_end = .;
        _bt_l2cap_fixed_chan_list_start = .;
        KEEP(*(SORT_BY_NAME("._bt_l2cap_fixed_chan.static.*")))
        _bt_l2cap_fixed_chan_list_end = .;
        _bt_l2cap_br_fixed_chan_list_start = .;
        KEEP(*(SORT_BY_NAME("._bt_l2cap_br_fixed_chan.static.*")))
        _bt_l2cap_br_fixed_chan_list_end = .;
        _bt_conn_cb_list_start = .;
        KEEP(*(SORT_BY_NAME("._bt_conn_cb.static.*")))
        _bt_conn_cb_list_end = .;
        _k_mem_slab_list_start = .;
        KEEP(*(SORT_BY_NAME("._k_mem_slab.static.*")))
        _k_mem_slab_list_end = .;

		_net_buf_pool_list = .;
       _net_buf_pool_list_start = .;
		KEEP(*(SORT_BY_NAME("._net_buf_pool.static.*")))
       _net_buf_pool_list_end = .;
       PROVIDE( _net_buf_pool_list = _net_buf_pool_list_start);
