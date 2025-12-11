void i2c_eeprom_bytewrite(u8 slaveAddr, u16 memAddr, u8 data);
u8   i2c_eeprom_randomread(u8 slaveAddr, u16 memAddr);
void i2c_eeprom_pagewrite(u8 slaveAddr, u16 memAddr, u8 *p, u8 nBytes);
void i2c_eeprom_sequentialread(u8 slaveAddr, u16 memAddr, u8 *p, u16 nBytes);



void i2c_eeprom_bytewrite(u8 slaveAddr, u16 memAddr, u8 data)
{
    i2c_start();
    i2c_write(slaveAddr << 1);                  // SLA + W

    i2c_write((memAddr >> 8) & 0xFF);           // HIGH address byte
    i2c_write(memAddr & 0xFF);                  // LOW address byte

    i2c_write(data);                            // Data byte
    i2c_stop();

    delay_ms(5);                                // Write cycle time
}

u8 i2c_eeprom_randomread(u8 slaveAddr, u16 memAddr)
{
    u8 rDat;

    i2c_start();
    i2c_write(slaveAddr << 1);                  // SLA + W

    i2c_write((memAddr >> 8) & 0xFF);           // HIGH address byte
    i2c_write(memAddr & 0xFF);                  // LOW address byte

    i2c_restart();
    i2c_write((slaveAddr << 1) | 1);            // SLA + R

    rDat = i2c_read_nack();
    i2c_stop();

    return rDat;
}



void i2c_eeprom_pagewrite(u8 slaveAddr, u16 memAddr, u8 *p, u8 nBytes)
{
    u8 i;

    i2c_start();
    i2c_write(slaveAddr << 1);

    i2c_write((memAddr >> 8) & 0xFF);           // HIGH address
    i2c_write(memAddr & 0xFF);                  // LOW address

    for(i = 0; i < nBytes; i++)
        i2c_write(p[i]);

    i2c_stop();
    delay_ms(5);
}

void i2c_eeprom_sequentialread(u8 slaveAddr, u16 memAddr, u8 *p, u16 nBytes)
{
    u16 i;

    i2c_start();
    i2c_write(slaveAddr << 1);

    i2c_write((memAddr >> 8) & 0xFF);           // HIGH address
    i2c_write(memAddr & 0xFF);                  // LOW address

    i2c_restart();
    i2c_write((slaveAddr << 1) | 1);

    for(i = 0; i < nBytes - 1; i++)
        p[i] = i2c_mack();

    p[i] = i2c_read_nack();
    i2c_stop();
}

