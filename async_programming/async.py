import asyncio
import time

import httpx


async def async_get(client: httpx.AsyncClient, url: str, **kwargs):
    try:
        resp = await client.request(method="GET", url=url, **kwargs)
        return resp.text
    except Exception as e:
        pass


async def evaluate_once():
    number_of_times = 20
    start_time = time.time()
    async with httpx.AsyncClient() as client:
        answers = await asyncio.gather(
            *(
                async_get(client, "http://www.example.org")
                for i in range(number_of_times)
            )
        )
    end_time = time.time()
    return end_time - start_time


async def evaluate(count: int):
    times = []
    print()
    for i in range(count):
        print(f"#################### Experiment {i} ####################")
        time = await evaluate_once()
        times.append(time)
        print(f"experiment {i} time: {time} seconds")
        print("######################################################")
        print("")
    mean_time = sum(times) / len(times)
    print("######################## Mean ########################")
    print(f"mean time in {count} experiments: {mean_time} seconds")
    print("######################################################")


if __name__ == "__main__":
    """
    Driver code
    """
    asyncio.get_event_loop().run_until_complete(evaluate(10))
