from pynput.keyboard import Key, Controller
import random
import time

kb = Controller()

lines = [
    "The quick brown fox jumps over the dog",
    "The quick brown fox jumps over the dog",
    "The quick brown fox jumps over the dog",
    "The quick brown fox jumps over the dog",
    "The quick brown fox jumps over the dog",
    "The quick brown fox jumps over the dog",
    "The quick brown fox jumps over the dog",
    "The quick brown fox jumps over the dog",
    "The quick brown fox jumps over the dog",
]

time.sleep(1)

def tap(key):
    global kb
    kb.press(key)
    kb.release(key)

for line in lines: 
    tap('o')
    i = 0
    while i < len(line):
        time.sleep(0.001)
        tap(line[i])
        if random.random() > 1 - 0.1: 
            for _ in range(random.randint(1, 8)):
                tap(Key.backspace)
                i -= 1
        if random.random() > 1 - 0.1: 
            tap(Key.esc)
            for _ in range(random.randint(1, 8)):
                tap('h')     
                i -= 1
       
        if i < 0: i = 0 
        i += 1
    tap(Key.esc)
