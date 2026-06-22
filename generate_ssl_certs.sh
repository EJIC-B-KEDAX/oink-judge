mkdir -p certs && cp -a certs certs.backup.$(date +%Y%m%d) 2>/dev/null || true

cat > certs/server_ext.cnf <<'EOF'
subjectAltName = @alt_names

[alt_names]
DNS.1 = localhost
DNS.2 = oink-judge.ru
IP.1 = 127.0.0.1
EOF

# 1. CA
openssl genrsa -out certs/ca.key 4096
openssl req -x509 -new -nodes -key certs/ca.key -sha256 -days 3650 \
  -out certs/ca.pem \
  -subj "/CN=OINK CA/O=OINK/L=Saint-Petersburg/ST=Saint-Petersburg/C=RU"

# 2. Server key + CSR
openssl genrsa -out certs/server.key 2048
openssl req -new -key certs/server.key -out certs/server.csr \
  -subj "/CN=localhost/O=OINK/L=Saint-Petersburg/ST=Saint-Petersburg/C=RU"

# 3. Server cert signed by CA
openssl x509 -req -in certs/server.csr \
  -CA certs/ca.pem -CAkey certs/ca.key -CAcreateserial \
  -out certs/server.crt -days 825 -sha256 \
  -extfile certs/server_ext.cnf

chmod 600 certs/ca.key certs/server.key
chmod 644 certs/ca.pem certs/server.crt