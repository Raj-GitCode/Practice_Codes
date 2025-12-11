//main.c
#include "system.h"     // All functions declared here
#include <LPC214x.h>

u8 tag[20];   // Global RFID buffer
/*
int main(void)
{
    System_Init();

    while(1)
    {
        RFID_ReadTag(tag);

        if(Check(tag))
        {
            if(VerifyPassword())
            {
                if(VerifyOTP())
                {
                    GrantAccess();
                    LED_ON();
                }
            }
        }
        else
        {
            ShowInvalidCard(tag);
        }
    }
}
*/

int main()
{
	System_Init();
	while(1)
		{
				if(password_update_request)   // If interrupt triggered
				{
						password_update_request = 0;  // Clear flag
						UpdatePassword();             // Allow password change
				}

				RFID_ReadTag(tag);

				if(Check(tag))
				{
						if(VerifyPassword())
						{
								if(VerifyOTP())
								{
										GrantAccess();
										LED_ON();
								}
						}
				}
				else
				{
						ShowInvalidCard(tag);
				}
		}

}
