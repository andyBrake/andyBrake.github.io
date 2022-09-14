import tkinter as tk

# 设置窗口大小变量
width = 1200
height = 700

player_name = 'A'
player_action = 'check'
player_info_format = "Player:%s\nRemind Bet:%d\nAction:%s"

global player_info 
global player_label
global bonus_info

player_info = []
player_label = []


def set_window_size(window):
    # 窗口居中，获取屏幕尺寸以计算布局参数，使窗口居屏幕中央
    screenwidth = window.winfo_screenwidth()
    screenheight = window.winfo_screenheight()
    print("W %d-%d H %d-%d"%((screenwidth-width)/2, screenwidth, (screenheight-height)/2, screenheight))
    size_geo = '%dx%d+%d+%d' % (width, height, (screenwidth-width)/2, (screenheight-height)/2)
    window.geometry(size_geo)
    #window.iconbitmap('C:/Users/Administrator/Desktop/favicon.ico')




def add_player(window):
    pixWidth = 300
    pixHeight = 200
    
    for i in range(0,8):
        info = tk.StringVar()
        info.set("Init")
        player_info.append(info)

        label = tk.Label(window, textvariable=info, bg="#7CCD7C",
                            # 设置标签内容区大小
                            width=15,height=4,
                            # 设置填充区距离、边框宽度和其样式（凹陷式）
                            padx=10, pady=10, borderwidth=10, relief="sunken")
        if i<4:
            x = i * pixWidth
        else:
            x = (i-4) * pixWidth
            
        y = 0 if (i<4) else pixHeight  
        
        label.place(x=x, y=y)
        player_label.append(label)
    return

# pubic card
def add_public_card(window):
    public_card_info = tk.StringVar()
    public_card_info.set(value= "\u2664 A   \u2661 K   \u2663 Q   \u2662 J   \u2663 10")

    label_card = tk.Label(window, textvariable = public_card_info, bg="#70CDFF",
                            # 设置标签内容区大小
                            width=35,height=2,
                            # 设置填充区距离、边框宽度和其样式（凹陷式）
                            padx=12, pady=12, borderwidth=5, relief="sunken")
    label_card.place(x=320, y=120)

# bonus label
def add_bonus(window):
    bonus_info = tk.StringVar()
    bonus_info.set(value=" 0 \u2664\u2661\u2663\u2662")

    label_bonus = tk.Label(window, textvariable = bonus_info, bg="#7CCDFF",
                            # 设置标签内容区大小
                            width=10,height=2,
                            # 设置填充区距离、边框宽度和其样式（凹陷式）
                            padx=12, pady=12, borderwidth=5, relief="sunken")
    label_bonus.place(x=700, y=120)

def update_player_info(player_id):
    global player_info
    name = player_name + " %d"%player_id
    player_info[player_id].set(player_info_format%(name, 500, "Raise \u2664\u2661\u2663\u2662"))

# 定义回调函数
def callback():
    for i in range(0, 8):
        update_player_info(i)
        #name = player_name + " %d"%i
        #player_info[i].set(player_info_format%(name, 500, "Raise \u2664\u2661\u2663\u2662"))  
    window.update()
    


if __name__ == '__main__':
    # 定义窗口
    window = tk.Tk()
    window.title('Poker')
    set_window_size(window)
    
    add_player(window)
    
    add_public_card(window)
    
    add_bonus(window)
    
    # 点击执行按钮
    button = tk.Button(window, text="执行", command=callback)
    button.place(x=0, y=650)
    window.mainloop()