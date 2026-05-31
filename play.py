from src import py_radio
import threading # new
import time

def stream_title(m):
  print("Song Title: ", m.decode("utf-8")," / Stream State: ",py_radio.stream_state())

def radio(u):
  i = 0

  while True:
    if py_radio.stream_state() != 1:
      py_radio.stream_start(u,stream_title)

      if i < 20:
        break
      i += 1

    print("Restarted!")
    time.sleep(5)

def main():
  thread = threading.Thread(target=radio, args=("https://mediaserv30.live-streams.nl:18079/stream",))
  thread.start()

  print("Started Radio!")

if __name__ == "__main__":
  main()