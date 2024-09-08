import requests
import time
import json

def fetch_json_with_retries(url, max_retries=5, timeout=5, backoff_factor=1):
    retries = 0

    while retries < max_retries:
        try:
            # Send the GET request
            response = requests.get(url, timeout=timeout)
            
            # Raise an error for non-200 status codes
            response.raise_for_status()

            # Validate if response contains valid JSON
            try:
                data = response.json()
                return data  # Successfully fetched and parsed JSON
            except json.JSONDecodeError:
                print("Invalid JSON response received.")
                return None

        except requests.exceptions.Timeout:
            print(f"Request timed out. Retrying... ({retries+1}/{max_retries})")

        except requests.exceptions.ConnectionError:
            print(f"Connection error. Retrying... ({retries+1}/{max_retries})")

        except requests.exceptions.HTTPError as e:
            print(f"HTTP error occurred: {e}")
            return None  # Don't retry on HTTP error

        except requests.exceptions.RequestException as e:
            print(f"An error occurred: {e}")
            return None  # Generic request exception

        # Increment retries and back off before retrying
        retries += 1
        sleep_time = backoff_factor * (2 ** retries)  # Exponential backoff
        print(f"Waiting {sleep_time} seconds before retrying...")
        time.sleep(sleep_time)

    print("Max retries reached. Failed to fetch data.")
    return None

# Main loop to continuously fetch JSON
url = 'http://192.168.1.10/json.html'
interval = 0.1  # Interval between fetch attempts

try:
    while True:
        print('==================== JSON REQUEST ======================')
        data = fetch_json_with_retries(url)

        if data:
            print("Successfully fetched JSON:", data)
        else:
            print("Failed to fetch JSON data.")

        # Wait before making the next request
        time.sleep(interval)

except KeyboardInterrupt:
    print("\nLoop interrupted. Exiting gracefully...")
    exit(0)
