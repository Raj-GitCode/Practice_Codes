void RFID_ReadTag(u8 *tag)
{
    CmdLCD(CLEAR_LCD);
    StrLCD("Show RFID Tag");

    /* Reset state before waiting for a new tag */
    r_flag = 0;
    r_index = 0;

    UART1_TxString((unsigned char *)"Waiting for RFID...\r\n");

    /* Wait for ISR to set r_flag == 2 (tag complete).
       This is blocking — you can replace with timeout if needed. */
    while(r_flag != 2) { /* idle */ }

    /* Copy received tag to provided buffer */
    strcpy((char*)tag, (char*)rfid_buff);

    /* Display on LCD */
    CmdLCD(CLEAR_LCD);
    StrLCD("RFID:");
    CmdLCD(0xC0);
    StrLCD((s8 *)tag);

    UART1_TxString((unsigned char *)"RFID: ");
    UART1_TxString((unsigned char *)tag);
    UART1_TxString((unsigned char *)"\r\n");
		
		
		CmdLCD(CLEAR_LCD);
    StrLCD("RFID TAG:");
    CmdLCD(0xC0);
    StrLCD((s8 *)tag);

    delay_s(2);   // show tag for 2 seconds
}
