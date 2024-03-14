#include <iostream>

#include "smtp_email.h"

#define CONF_PATH "/Code/Koro_Caster/app/smtp_email_demo/smtp.conf"

int main()
{

    smtp_email *smtp = new smtp_email();

    smtp->load_conf(CONF_PATH);

    smtp->start();


    smtp->set_period_msg("this is a period msg");

    sleep(1);

    for (int i = 1; i < 5; i++)
    {
        smtp->send_temp_msg("<br />啊这<br />");

        sleep(i * 120);

        std::string msg = std::to_string(i * 30);

        smtp->set_period_msg(msg.c_str());
    }

    return 0;
}