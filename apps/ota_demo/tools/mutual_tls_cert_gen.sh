#!/bin/bash
set -e

echo
echo "==============================="
echo "Mutual TLS Certificate Generation Script"
echo "==============================="
echo

# === Ask for CA name ===
read -p "Enter a name for the CA certificate and key files (default: ca): " CA_NAME
CA_NAME=${CA_NAME:-ca}
CA_KEY="${CA_NAME}.key"
CA_CERT="${CA_NAME}-cert.pem"
CA_SRL="${CA_NAME}-cert.srl"

# === Ask user which certs to generate ===
read -p "Do you want to generate the SERVER certificate? (y/n, default: y): " server_choice
server_choice=${server_choice:-y}
if [[ "$server_choice" =~ ^[Yy] ]]; then
    GENERATE_SERVER_CERT=true
else
    GENERATE_SERVER_CERT=false
fi

read -p "Do you want to generate the CLIENT certificate? (y/n, default: y): " client_choice
client_choice=${client_choice:-y}
if [[ "$client_choice" =~ ^[Yy] ]]; then
    GENERATE_CLIENT_CERT=true
else
    GENERATE_CLIENT_CERT=false
fi

echo
echo "--- Generating CA ---"
if [ -f "$CA_KEY" ]; then
    read -p "File '$CA_KEY' already exists. Overwrite? (y/n, default: y): " OVERWRITE
    OVERWRITE=${OVERWRITE:-y}
    if [[ "$OVERWRITE" =~ ^[Yy] ]]; then
        openssl genrsa -out "$CA_KEY" 2048
    else
        echo "Skipping CA key generation."
    fi
else
    openssl genrsa -out "$CA_KEY" 2048
fi

if [ -f "$CA_CERT" ]; then
    read -p "File '$CA_CERT' already exists. Overwrite? (y/n, default: y): " OVERWRITE
    OVERWRITE=${OVERWRITE:-y}
    if [[ "$OVERWRITE" =~ ^[Yy] ]]; then
        export MSYS_NO_PATHCONV=1
        openssl req -x509 -new -nodes -key "$CA_KEY" -sha256 -days 3650 -out "$CA_CERT" -subj "/C=US/ST=California/L=San Francisco/O=MyOrg/OU=MyOrgUnit/CN=MyRootCA"
    else
        echo "Skipping CA certificate generation."
    fi
else
    export MSYS_NO_PATHCONV=1
    openssl req -x509 -new -nodes -key "$CA_KEY" -sha256 -days 3650 -out "$CA_CERT" -subj "/C=US/ST=California/L=San Francisco/O=MyOrg/OU=MyOrgUnit/CN=MyRootCA"
fi

echo
# === Generate SERVER certificate ===
if [ "$GENERATE_SERVER_CERT" = true ]; then
    echo "--- Generating SERVER certificate ---"
    read -p "Enter a name for the SERVER certificate files (default: mutual-server): " SERVER_NAME
    SERVER_NAME=${SERVER_NAME:-mutual-server}
    SERVER_KEY="${SERVER_NAME}.key"
    SERVER_CSR="${SERVER_NAME}.csr"
    SERVER_CERT="${SERVER_NAME}-cert.pem"
    SERVER_CNF="${SERVER_NAME}-cert.cnf"

    if [ -f "$SERVER_KEY" ]; then
        read -p "File '$SERVER_KEY' already exists. Overwrite? (y/n, default: y): " OVERWRITE
        OVERWRITE=${OVERWRITE:-y}
        if [[ "$OVERWRITE" =~ ^[Yy] ]]; then
            openssl genrsa -out "$SERVER_KEY" 2048
        else
            echo "Skipping SERVER key generation."
        fi
    else
        openssl genrsa -out "$SERVER_KEY" 2048
    fi

    # Get SERVER IP input, default to 0.0.0.0 if empty
    read -p "Enter the SERVER IP for the certificate (default: 0.0.0.0): " SERVER_IP
    SERVER_IP=${SERVER_IP:-0.0.0.0}

    if [ -f "$SERVER_CSR" ]; then
        read -p "File '$SERVER_CSR' already exists. Overwrite? (y/n, default: y): " OVERWRITE
        OVERWRITE=${OVERWRITE:-y}
        if [[ "$OVERWRITE" =~ ^[Yy] ]]; then
            export MSYS_NO_PATHCONV=1
            openssl req -new -key "$SERVER_KEY" -out "$SERVER_CSR" -subj "/C=US/ST=California/L=San Francisco/O=MyOrg/OU=MyOrgUnit/CN=$SERVER_IP"
        else
            echo "Skipping SERVER CSR generation."
        fi
    else
        export MSYS_NO_PATHCONV=1
        openssl req -new -key "$SERVER_KEY" -out "$SERVER_CSR" -subj "/C=US/ST=California/L=San Francisco/O=MyOrg/OU=MyOrgUnit/CN=$SERVER_IP"
    fi

    # Create server config file for SAN
    echo "[ v3_ca ]
subjectAltName = @alt_names

[alt_names]
IP.1 = $SERVER_IP" > "$SERVER_CNF"

    if [ -f "$SERVER_CERT" ]; then
        read -p "File '$SERVER_CERT' already exists. Overwrite? (y/n, default: y): " OVERWRITE
        OVERWRITE=${OVERWRITE:-y}
        if [[ "$OVERWRITE" =~ ^[Yy] ]]; then
            openssl x509 -req -in "$SERVER_CSR" -CA "$CA_CERT" -CAkey "$CA_KEY" -CAcreateserial -out "$SERVER_CERT" -days 365 -sha256 -extfile "$SERVER_CNF" -extensions v3_ca
        else
            echo "Skipping SERVER certificate signing."
        fi
    else
        openssl x509 -req -in "$SERVER_CSR" -CA "$CA_CERT" -CAkey "$CA_KEY" -CAcreateserial -out "$SERVER_CERT" -days 365 -sha256 -extfile "$SERVER_CNF" -extensions v3_ca
    fi
    echo
fi

# === Generate CLIENT certificate ===
if [ "$GENERATE_CLIENT_CERT" = true ]; then
    echo "--- Generating CLIENT certificate ---"
    read -p "Enter a name for the CLIENT certificate files (default: mutual-client): " CLIENT_NAME
    CLIENT_NAME=${CLIENT_NAME:-mutual-client}
    CLIENT_KEY="${CLIENT_NAME}.key"
    CLIENT_CSR="${CLIENT_NAME}.csr"
    CLIENT_CERT="${CLIENT_NAME}-cert.pem"
    CLIENT_CNF="${CLIENT_NAME}-cert.cnf"

    if [ -f "$CLIENT_KEY" ]; then
        read -p "File '$CLIENT_KEY' already exists. Overwrite? (y/n, default: y): " OVERWRITE
        OVERWRITE=${OVERWRITE:-y}
        if [[ "$OVERWRITE" =~ ^[Yy] ]]; then
            openssl genrsa -out "$CLIENT_KEY" 2048
        else
            echo "Skipping CLIENT key generation."
        fi
    else
        openssl genrsa -out "$CLIENT_KEY" 2048
    fi

    # Validate CLIENT DNS and IP input
    while true; do
        read -p "Enter the CLIENT DNS name (default: device01): " CLIENT_DNS
        CLIENT_DNS=${CLIENT_DNS:-device01}
        if [[ -z "$CLIENT_DNS" ]]; then
            echo "CLIENT DNS cannot be empty. Please enter a valid DNS name."
        else
            break
        fi
    done
    while true; do
        read -p "Enter the CLIENT IP (default: 192.168.112.205): " CLIENT_IP
        CLIENT_IP=${CLIENT_IP:-192.168.112.205}
        if [[ -z "$CLIENT_IP" ]]; then
            echo "CLIENT IP cannot be empty. Please enter a valid IP."
        else
            break
        fi
    done

    if [ -f "$CLIENT_CSR" ]; then
        read -p "File '$CLIENT_CSR' already exists. Overwrite? (y/n, default: y): " OVERWRITE
        OVERWRITE=${OVERWRITE:-y}
        if [[ "$OVERWRITE" =~ ^[Yy] ]]; then
            export MSYS_NO_PATHCONV=1
            openssl req -new -key "$CLIENT_KEY" -out "$CLIENT_CSR" -subj "/C=US/ST=California/L=San Francisco/O=MyOrg/OU=MyOrgUnit/CN=$CLIENT_DNS"
        else
            echo "Skipping CLIENT CSR generation."
        fi
    else
        export MSYS_NO_PATHCONV=1
        openssl req -new -key "$CLIENT_KEY" -out "$CLIENT_CSR" -subj "/C=US/ST=California/L=San Francisco/O=MyOrg/OU=MyOrgUnit/CN=$CLIENT_DNS"
    fi

    # Create client config file for SAN
    echo "[ v3_ca ]
subjectAltName = @alt_names

[alt_names]
DNS.1 = $CLIENT_DNS
IP.1 = $CLIENT_IP" > "$CLIENT_CNF"

    if [ -f "$CLIENT_CERT" ]; then
        read -p "File '$CLIENT_CERT' already exists. Overwrite? (y/n, default: y): " OVERWRITE
        OVERWRITE=${OVERWRITE:-y}
        if [[ "$OVERWRITE" =~ ^[Yy] ]]; then
            openssl x509 -req -in "$CLIENT_CSR" -CA "$CA_CERT" -CAkey "$CA_KEY" -CAcreateserial -out "$CLIENT_CERT" -days 365 -sha256 -extfile "$CLIENT_CNF" -extensions v3_ca
        else
            echo "Skipping CLIENT certificate signing."
        fi
    else
        openssl x509 -req -in "$CLIENT_CSR" -CA "$CA_CERT" -CAkey "$CA_KEY" -CAcreateserial -out "$CLIENT_CERT" -days 365 -sha256 -extfile "$CLIENT_CNF" -extensions v3_ca
    fi
    echo
fi

echo "==============================="
echo "Certificate generation complete."
echo "==============================="
echo
read -p "Script finished. Press Enter to exit."
