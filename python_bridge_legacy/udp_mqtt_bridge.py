import socket
from tdmclient import ClientAsync, aw

def main():
    # UDP Socket Configuration
    UDP_IP = "0.0.0.0"
    UDP_PORT = 5000
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((UDP_IP, UDP_PORT))
    
    print(f"UDP Bridge Server listening on port {UDP_PORT}...")

    # Connection to the Thymio TDM (Thymio Device Manager)
    print("Connecting to Thymio...")
    client = ClientAsync()
    node = aw(client.wait_for_node())
    aw(node.lock())
    print("Thymio connected and locked! Ready to receive commands.")

    try:
        while True:
            # Receive UDP payload from ESP32
            data, addr = sock.recvfrom(1024)
            comando = data.decode('utf-8').strip()
            print(f"Received command: {comando} from {addr}")

            # Translate the received string into physical actions (LEDs and Motors)
            if comando == "ROSSO":
                aw(node.set_variables({"leds.top": [32, 0, 0]}))
                aw(node.set_variables({"motor.left.target": [0], "motor.right.target": [0]}))
                
            elif comando == "VERDE":
                aw(node.set_variables({"leds.top": [0, 32, 0]}))
                aw(node.set_variables({"motor.left.target": [200], "motor.right.target": [200]}))
                
            elif comando == "BLU":
                aw(node.set_variables({"leds.top": [0, 0, 32]}))
                aw(node.set_variables({"motor.left.target": [200], "motor.right.target": [-200]}))
                
            elif comando == "BIANCO":
                aw(node.set_variables({"leds.top": [32, 32, 32]}))
                aw(node.set_variables({"motor.left.target": [-150], "motor.right.target": [-150]}))
                
            elif comando == "NERO":
                aw(node.set_variables({"leds.top": [0, 0, 0]}))
                aw(node.set_variables({"motor.left.target": [0], "motor.right.target": [0]}))

    except KeyboardInterrupt:
        print("\nShutting down bridge...")
        # Stop the robot before exiting
        aw(node.set_variables({"motor.left.target": [0], "motor.right.target": [0], "leds.top": [0, 0, 0]}))
        aw(node.unlock())
        sock.close()

if __name__ == "__main__":
    main()