	/* TEST: PHY functions */
	for (i = 0; i < WORK_BUF_LEN; i++)
		send_test_work(i);

while(1) {
	while (alink_rxbuf_empty())
		;

	alink_buf_status();
	alink_read_result(&result);
	submit_result(&result);
	alink_buf_status();
}
