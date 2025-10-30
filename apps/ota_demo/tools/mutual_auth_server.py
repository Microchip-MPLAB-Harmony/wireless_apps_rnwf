import os
import socket
import ssl
import math

# Macro to select authentication mode
MUTUAL_AUTH_ENABLED = True  # Set to True for mutual TLS, False for server-only TLS

# Certificate and key file paths
# Mutual TLS certificates
MUTUAL_SERVER_CERT = "mutual-server-cert.pem"
MUTUAL_SERVER_KEY  = "mutual-server.key"
CA_CERT            = "ca-cert.pem"

# Server-only TLS certificates
SERVER_CERT        = "server-cert.pem"
SERVER_KEY         = "server-key.pem"

# Server configuration
HOST = '0.0.0.0'  # Listen on all interfaces
PORT = 443

# Create a socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)  # Allow reuse of the address
server_socket.bind((HOST, PORT))
server_socket.listen(5)

print(f"Server listening on {HOST}:{PORT}")

# SSL context setup
if MUTUAL_AUTH_ENABLED:
    # Mutual TLS authentication
    context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
    context.load_cert_chain(certfile=MUTUAL_SERVER_CERT, keyfile=MUTUAL_SERVER_KEY)
    context.load_verify_locations(cafile=CA_CERT)  # CA cert to verify client certs
    context.verify_mode = ssl.CERT_REQUIRED  # Require client certificate
    print("Mutual TLS authentication enabled.")
else:
    # Server-only authentication
    context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
    context.load_cert_chain(certfile=SERVER_CERT, keyfile=SERVER_KEY)
    # No client certificate required
    context.verify_mode = ssl.CERT_NONE
    print("Server-only TLS authentication enabled.")

def send_file_in_chunks(client_socket, response_header, file_path):
    total_size = os.path.getsize(file_path)
    sent_size = 0
    chunk_size = 4096
    next_percent = 10

    # Send the response header first
    client_socket.sendall(response_header)

    with open(file_path, 'rb') as file:
        while True:
            chunk = file.read(chunk_size)
            if not chunk:
                break
            client_socket.sendall(chunk)
            sent_size += len(chunk)
            percent = int((sent_size / total_size) * 100)
            if percent >= next_percent:
                print(f"Sent {next_percent}% of {file_path} ({sent_size}/{total_size} bytes)")
                next_percent += 10
                if next_percent > 100:
                    next_percent = 100

def handle_request(client_socket):
    try:
        # Receive the request
        request = client_socket.recv(1024).decode('utf-8')
        print(f"Received request: {request}")

        # Parse the request
        headers = request.split('\n')
        if headers:
            method, path, _ = headers[0].split()
            if method == 'GET':
                # Remove leading slash
                if path == '/':
                    path = '/index.html'  # Default to index.html
                file_path = path.lstrip('/')

                # Serve the file
                if os.path.isfile(file_path):
                    total_size = os.path.getsize(file_path)
                    print(f"Sending file: {file_path}, Length: {total_size} bytes")
                    response_header = (
                        "HTTP/1.1 200 OK\r\n"
                        f"Content-Length: {total_size}\r\n"
                        "Content-Type: application/octet-stream\r\n"
                        "\r\n"
                    ).encode('utf-8')
                    send_file_in_chunks(client_socket, response_header, file_path)
                else:
                    print(f"File not found: {file_path}")
                    response = (
                        "HTTP/1.1 404 Not Found\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: 13\r\n"
                        "\r\n"
                        "404 Not Found"
                    ).encode('utf-8')
                    client_socket.sendall(response)
    except Exception as e:
        print(f"Error handling request: {e}")
    finally:
        # Close the connection
        client_socket.close()

try:
    with context.wrap_socket(server_socket, server_side=True) as tls_server_socket:
        tls_server_socket.settimeout(1.0)  # 1 second timeout for accept()
        while True:
            try:
                client_socket, addr = tls_server_socket.accept()
                print(f"Connection from {addr}")
                handle_request(client_socket)
            except ssl.SSLError as ssl_err:
                print(f"SSL error: {ssl_err}")
            except socket.timeout:
                continue  # Allows KeyboardInterrupt to be caught
except KeyboardInterrupt:
    print("\nServer terminated by user (Ctrl+C).")
finally:
    server_socket.close()
