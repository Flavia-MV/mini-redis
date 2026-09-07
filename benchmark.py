import socket
import time
import threading

HOST="localhost"
PORT=6379
NUM_THREADS = 10
COMMANDS_PER_THREAD=1000

def worker(thread_id, results):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, PORT))
    sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

    for i in range(COMMANDS_PER_THREAD):
        command = f"SET thread{thread_id}_key{i} value{i}\n"
        sock.sendall(command.encode())
        sock.recv(1024)

    sock.close()
    results[thread_id] = COMMANDS_PER_THREAD

def run_concurrent_benchmark():
    threads = []
    results = {}
    start_time = time.time()

    for t_id in range(NUM_THREADS):
        t = threading.Thread(target = worker, args = (t_id, results))
        threads.append(t)
        t.start()

    for t in threads:
        t.join()

    end_time = time.time()

    total_commands = sum(results.values())
    total_time = end_time - start_time
    ops_per_sec = total_commands / total_time

    print(f"Executed {total_commands} SET commands using {NUM_THREADS} concurrent clients in {total_time:.3f} seconds")
    print(f"Throughput: {ops_per_sec:.0f} ops/sec")

if __name__ == "__main__":
    run_concurrent_benchmark()