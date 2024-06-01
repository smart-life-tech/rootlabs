import bcrypt

# The hashed password (the one you have)
hashed_password = b"$2b$10$Xo/4YfOAzNdIVtl7RRTAa.lQJ4PS6rPhyoOVS3Zvj24fwEptvQnZO"

# The plaintext password you want to verify
plaintext_password = "peter"  # replace with the password you want to test

# Convert plaintext password to bytes
plaintext_password_bytes = plaintext_password.encode('utf-8')

# Verify the password
if bcrypt.checkpw(plaintext_password_bytes, hashed_password):
    print("Password matches!")
else:
    print("Password does not match.")
