#include "base64.h"






char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *util_base64_encode(char *message)
{
	char *encoded;
	unsigned long length, encoded_length;
	unsigned long left, bitqueue, i = 0, j = 0;

	length = strlen(message);

	if (length == 0) return NULL;

	encoded_length = (4 * (length + ((3 - (length % 3)) % 3)) / 3);
	encoded = (char *)malloc(encoded_length + 1);

	while (i < length) {
		left = length - i;

		if (left > 2) {
			bitqueue = message[i++];
			bitqueue = (bitqueue << 8) + message[i++];
			bitqueue = (bitqueue << 8) + message[i++];
			
			encoded[j++] = alphabet[(bitqueue & 0xFC0000) >> 18];
			encoded[j++] = alphabet[(bitqueue & 0x3F000) >> 12];
			encoded[j++] = alphabet[(bitqueue & 0xFC0) >> 6];
			encoded[j++] = alphabet[bitqueue & 0x3F];
		} else if (left == 2) {
			bitqueue = message[i++];
			bitqueue = (bitqueue << 8) + message[i++];
			bitqueue <<= 8;

			encoded[j++] = alphabet[(bitqueue & 0xFC0000) >> 18];
			encoded[j++] = alphabet[(bitqueue & 0x3F000) >> 12];
			encoded[j++] = alphabet[(bitqueue & 0xFC0) >> 6];
			encoded[j++] = '=';			
		} else {
			bitqueue = message[i++];
			bitqueue <<= 16;

			encoded[j++] = alphabet[(bitqueue & 0xFC0000) >> 18];
			encoded[j++] = alphabet[(bitqueue & 0x3F000) >> 12];
			encoded[j++] = '=';
			encoded[j++] = '=';
		}
	}
	
	encoded[encoded_length] = '\0'; // added. ajd

	return encoded;
}

char unalphabet(char alpha)
{
	if (alpha >= 'A' && alpha <= 'Z')
		return (alpha - 'A');
	else if (alpha >= 'a' && alpha <= 'z')
		return (alpha - 'a' + 26);
	else if (alpha >= '0' && alpha <= '9')
		return (alpha - '0' + 52);
	else if (alpha == '+')
		return 62;
	else if (alpha == '/')
		return 63;
	else if (alpha == '=')
		return 64;
	else 
		return 65;
}

char *util_base64_decode(char *message)
{
	char *decoded, temp;
	long length, decoded_length;
	long bitqueue, pad, i = 0, j = 0;

	length = strlen(message);

	if (((length % 4) != 0) || (length == 0)) return NULL;

	decoded_length = length / 4 * 3;

	if (message[length - 1] == '=') {
		decoded_length--;
		if (message[length - 2] == '=')
			decoded_length--;
	}

	decoded = (char *)malloc(decoded_length + 1);
	memset (decoded, 0, decoded_length + 1);

	while (i < length) {
		pad = 0;

		temp = unalphabet(message[i++]);
		if (temp == 64) {
			free(decoded);
			return NULL;
		}
		bitqueue = temp;

		temp = unalphabet(message[i++]);
		if (temp == 64) {
			free(decoded);
			return NULL;
		}
		bitqueue <<= 6;
		bitqueue += temp;

		temp = unalphabet(message[i++]);
		if (temp == 64) {
			if (i != length - 1) {
				free(decoded);
				return NULL;
			}
			temp = 0; pad++;
		}
		bitqueue <<= 6;
		bitqueue += temp;

		temp = unalphabet(message[i++]);
		if (pad == 1 && temp != 64) {
				free(decoded);
				return NULL;
		}
		
		if (temp == 64) {
			if (i != length) {
				free(decoded);
				return NULL;
			}
			temp = 0; pad++;
		}
		bitqueue <<= 6;
		bitqueue += temp;

		decoded[j++] = ((bitqueue & 0xFF0000) >> 16);
		if (pad < 2) {
			decoded[j++] = ((bitqueue & 0xFF00) >> 8);
			if (pad < 1)
				decoded[j++] = (bitqueue & 0xFF);
		}
	}
	
	decoded[decoded_length] = '\0'; // added. ajd

	return decoded;
}