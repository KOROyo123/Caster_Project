#pragma once
#include <string>
#include <vector>
#include <map>

#include <openssl/ossl_typ.h>

#ifndef WIN32
#include<netdb.h>
#else
#include <WS2tcpip.h>
#endif

class SmtpBase
{
protected:
	struct EmailInfo
	{
		std::string smtpServer;      //the SMTP server
		std::string serverPort;      //the SMTP server port
		std::string charset;         //the email character set
		std::string sender;          //the sender's name
		std::string senderEmail;     //the sender's email
		std::string password;        //the password of sender
		std::string recipient;       //the recipient's name
		std::string recipientEmail;  //the recipient's email

		std::map<std::string, std::string> recvList; //�ռ����б�<email, name>

		std::string subject;         //the email message's subject  �ʼ�����
		std::string message;         //the email message body   �ʼ�����

		std::map<std::string, std::string> ccEmail;         //�����б�
		std::vector<std::string> attachment; //����
	};
public:

	virtual ~SmtpBase() {}
	/**
	 * @brief �򵥷����ı��ʼ�
	 * @param   from �����ߵ��ʺ�
	 * @param   passs ����������
	 * @param   to �ռ���
	 * @param   subject ����
	 * @param   strMessage  �ʼ�����
	 */

	virtual int SendEmail(const std::string& from, const std::string& passs, const std::string& to, const std::string& subject, const std::string& strMessage) = 0;
	/**
	 * @brief �����ʼ������������Լ������˺Ͷ���ռ���
	 * @param   from �����ߵ��ʺ�
	 * @param   passs ����������
	 * @param   vecTo �ռ����б�
	 * @param   subject ����
	 * @param   strMessage  �ʼ�����
	 * @param   attachment  �����б�    ���������Ǿ���·����Ĭ���ǿ�ִ�г���Ŀ¼��
	 * @param   ccList  �����б�
	 */
	virtual int SendEmail(const std::string& from, const std::string& passs, const std::vector<std::string>& vecTo,
		const std::string& subject, const std::string& strMessage, const std::vector<std::string>& attachment, const std::vector<std::string>& ccList) = 0;

	std::string GetLastError()
	{
		return m_lastErrorMsg;
	}

	virtual int Read(void* buf, int num) = 0;
	virtual int Write(const void* buf, int num) = 0;
	virtual int Connect() = 0;
	virtual int DisConnect() = 0;

protected:

	std::string m_lastErrorMsg;


};


class SmtpEmail : public SmtpBase
{

public:
	SmtpEmail(const std::string& emailHost, const std::string& port);
	~SmtpEmail();

	int SendEmail(const std::string& from, const std::string& passs, const std::string& to, const std::string& subject, const std::string& strMessage);

	int SendEmail(const std::string& from, const std::string& passs, const std::vector<std::string>& vecTo,
		const std::string& subject, const std::string& strMessage, const std::vector<std::string>& attachment, const std::vector<std::string>& ccList);
protected:
	int Read(void* buf, int num);
	int Write(const void* buf, int num);
	int Connect();
	int DisConnect();

	virtual std::string GetEmailBody(const EmailInfo & info);
private:
	//int SMTPSSLComunicate(SSL *connection, const EmailInfo &info);
	int SMTPComunicate(const EmailInfo &info);

protected:
	addrinfo* m_addrinfo;
	int m_socketfd;

	std::string m_host;
	std::string m_port;

	bool m_isConnected;
};

class SimpleSmtpEmail : public SmtpEmail
{
public:
	using SmtpEmail::SmtpEmail;
	virtual std::string GetEmailBody(const EmailInfo & info);
};

class SslSmtpEmail : public SmtpEmail
{
public:
	using SmtpEmail::SmtpEmail;
	SslSmtpEmail(const std::string& emailHost, const std::string& port) :SmtpEmail(emailHost, port) {};
	~SslSmtpEmail();

	int Connect();
	int DisConnect();
protected:
	int Read(void* buf, int num);
	int Write(const void* buf, int num);
private:
	SSL_CTX *m_ctx;
	SSL *m_ssl;
};

class SimpleSslSmtpEmail : public SslSmtpEmail
{
public:
	using SslSmtpEmail::SslSmtpEmail;
	SimpleSslSmtpEmail(const std::string& emailHost, const std::string& port) :SslSmtpEmail(emailHost, port) {};
	virtual std::string GetEmailBody(const EmailInfo & info);
};
