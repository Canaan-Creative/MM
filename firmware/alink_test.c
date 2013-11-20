	/* TEST: PHY functions */
	for (i = 0; i < WORK_BUF_LEN; i++)
		send_test_work(i);

	error(0xf);
	error(0xf);
	error(0xf);

	while (alink_busy_status())
		;

	alink_buf_status();
	alink_read_result(&result);
	submit_result(&result);
	alink_buf_status();
	alink_read_result(&result);
	submit_result(&result);
	alink_buf_status();

	while(1);
