from concurrent.futures import ThreadPoolExecutor
from queue import Queue
import requests
import time
import threading


print_queue = Queue()
print_worker_event = threading.Event()


def print_worker():
    while True:
        if print_worker_event.is_set() and print_queue.empty():
            break
        element = print_queue.get()
        print(element, flush=True)


def get(url: str, rid: int):
    response = requests.get(url)
    # print_queue.put(f"request {rid}: {response.status_code}")


def main(parallel_req_count: int):
    with ThreadPoolExecutor(max_workers=20) as executor:
        for i in range(parallel_req_count):
            executor.submit(get, "http://example.com", i)


def evaluate_once() -> float:
    start = time.time()
    main(20)
    end = time.time()
    return end - start


def evaluate(count: int):
    times = []
    print()
    for i in range(count):
        print_queue.put(f"#################### Experiment {i} ####################")
        time = evaluate_once()
        times.append(time)
        print_queue.put(f"experiment {i} time: {time} seconds")
        print_queue.put("######################################################")
        print_queue.put("")
    mean_time = sum(times) / len(times)
    print_queue.put("######################## Mean ########################")
    print_queue.put(f"mean time in {count} experiments: {mean_time} seconds")
    print_queue.put("######################################################")


if __name__ == "__main__":
    screen_printing_thread = threading.Thread(target=print_worker)
    screen_printing_thread.start()
    evaluate(10)
    print_worker_event.set()
