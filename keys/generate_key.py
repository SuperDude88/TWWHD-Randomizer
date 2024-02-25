import secrets

key_1 = int(secrets.token_hex(16), 16)
key_2 = int(secrets.token_hex(16), 16)
key_3 = int(secrets.token_hex(16), 16)
key_4 = int(secrets.token_hex(16), 16)
print(str(key_1 - (key_2 + key_3) * key_4))
