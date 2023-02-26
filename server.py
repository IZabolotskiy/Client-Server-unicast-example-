import socket
import struct
import thread 

SERVER_ADDRESS = 'localhost'
SERVER_PORT = 8000
LOG_FILE = 'position_log.txt'

def handle_client(client_socket):
    # Handle messages from the client here
    while True:
        data = client_socket.recv(12)
        if not data:
            break
        # Interpret the message as a 3-axis position
        x, y, z = struct.unpack('fff', data)
        position = f'{x},{y},{z}'
        print(f'Received position: ({x}, {y}, {z})')
        # Write the position to the log file
        with open(LOG_FILE, 'a') as f:
            f.write(position + '\n')
    client_socket.close()

def run_server():
    # Create a socket and bind it to the server address and port
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((SERVER_ADDRESS, SERVER_PORT))
    server_socket.listen()

    print(f'Server listening on {SERVER_ADDRESS}:{SERVER_PORT}')

    # Loop to accept new client connections and handle them in a new thread
    while True:
        client_socket, client_address = server_socket.accept()
        print(f'New client connected: {client_address}')
        client_thread = threading.Thread(target=handle_client, args=(client_socket,))
        client_thread.start()

if __name__ == '__main__':
    run_server()