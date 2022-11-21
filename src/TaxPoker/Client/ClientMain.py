from unicodedata import name
from ClientController import *
from ClientModule import *
from ClientView import *

if __name__ == '__main__':
    player = Player(name = "Fa", id=-1, total_bet = 500)

    controller = ClientController(p=player, ip='127.0.0.1',port=10002)

    controller.start()
 
    loop = 20
    while True:
        loop = loop - 1
        sleep(1)
        if loop == 0:
            controller.stop()
            break


