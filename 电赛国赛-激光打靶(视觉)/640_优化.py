import time, os, sys
from media.sensor import *
from media.display import *
from media.media import *
from ybUtils.YbKey import YbKey
from ybUtils.YbUart import YbUart

key = YbKey()
uart = YbUart(baudrate=115200)
#最下面的tx
# 显示参数 / Display parameters
DISPLAY_WIDTH = 640    # LCD显示宽度 / LCD display width
DISPLAY_HEIGHT = 480   # LCD显示高度 / LCD display height
BLACK_THRESHOLD =(0,76)#可调,第一最小小阈值
BLACK_THRESHOLD2 =(0,100)#可调,第二阈值
BLACK_THRESHOLD3 =(0,100)#可调,第三阈值



# 定义色块检测参数
MIN_PIXELS = 11     # 最小像数（过滤噪声）
MAX_PIXELS = 75000   # 最大像素数（过滤大面积区域）
AREA_THRESHOLD = 50  # 区域面积阈值（可选）
clock = time.clock()

max_density=0.22
min_density=0.1
max_solidity=0.31
max_convexity=0.63
min_convexity=0.45
min_area=4000
max_area=73000
data = bytearray([0x55,0,0,0,0,0,0,0,0])

flag=0
target_x_2=0
target_y_2=0
ccx=0
ccy=0#这四个变量是让识别不到时也保证发送上一次识别到的变量
# 在main()函数开头添加全局变量初始化
target_x = 306  # 默认坐标
target_y = 275
intersection = (0, 0)  # 默认交点坐标
flag = 0  # 检测标志位
flag1=0
flag2=0
flags=0
def sort_corners(pts):
    #将四个角点排序为 (左上, 右上, 右下, 左下)
    #pts: [(x1,y1),(x2,y2),(x3,y3),(x4,y4)]
    #按 y 排序，取出上下两组
    pts = sorted(pts, key=lambda p: (p[1], p[0]))
    top = sorted(pts[:2], key=lambda p: p[0])   #上边两个按 x 排序
    bottom = sorted(pts[2:], key=lambda p: p[0])#下边两个按 x 排序
    return [top[0], top[1], bottom[1], bottom[0]]
def solve_linear(A, b):
    #简易高斯消元 Ax=b
    n = len(A)
    for i in range(n):
        A[i] = A[i] + [b[i]]
    for i in range(n):
        # 选主元
        max_row = max(range(i, n), key=lambda r: abs(A[r][i]))
        A[i], A[max_row] = A[max_row], A[i]
        # 归一化主元
        div = A[i][i]
        for j in range(i, n+1):
            A[i][j] /= div
        # 消元
        for r in range(n):
            if r != i:
                factor = A[r][i]
                for j in range(i, n+1):
                    A[r][j] -= factor * A[i][j]
    return [A[i][n] for i in range(n)]


def get_perspective_transform(src_pts, dst_pts):
    #求透视矩阵 H
    A, b = [], []
    for (x,y), (X,Y) in zip(src_pts, dst_pts):
        A.append([x, y, 1, 0, 0, 0, -X*x, -X*y])
        b.append(X)
        A.append([0, 0, 0, x, y, 1, -Y*x, -Y*y])
        b.append(Y)
    h = solve_linear(A, b)
    h.append(1.0)
    H = [[h[0], h[1], h[2]],
         [h[3], h[4], h[5]],
         [h[6], h[7], h[8]]]
    return H

def warp_point(H, x, y):
    """用透视矩阵 H 映射点"""
    X = H[0][0]*x + H[0][1]*y + H[0][2]
    Y = H[1][0]*x + H[1][1]*y + H[1][2]
    W = H[2][0]*x + H[2][1]*y + H[2][2]
    return int(X/W), int(Y/W)

WIDTH = 640# 定义图像宽度常量
HEIGHT = 480# 定义图像高度常量
def line_intersection(line1, line2):
    """
    计算两条直线的交点坐标
    """
    (x1, y1, x2, y2) = line1
    (x3, y3,x4, y4) = line2

    # 计算分母（判断是否平行）
    den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4)
    if den == 0:
        # 分母为0，两直线平行或重合，无唯一交点
        return (abs(x2-x1)//2,abs(y2-y1)//2)

    # 计算分子
    t_num = (x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)
    s_num = (x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2)

    t = t_num / den
    s = s_num / den

    # 计算交点坐标
    x = x1 + t * (x2 - x1)
    y = y1 + t * (y2 - y1)

    return (x, y)
def init_sensor():
    """初始化摄像头 / Initialize camera sensor"""
    sensor = Sensor()
    sensor.reset()
    sensor.set_framesize(width=DISPLAY_WIDTH, height=DISPLAY_HEIGHT)
    sensor.set_pixformat(sensor.GRAYSCALE)  # 设置为灰度模式
    return sensor

def init_display():
    """初始化显示 / Initialize display"""
    Display.init(Display.ST7701,  width = DISPLAY_WIDTH, height = DISPLAY_HEIGHT,to_ide=True)
    MediaManager.init()

def blob_1(img):
    print("执行小阈值识别")
    flag=0
    # 查找符合阈值的色块
    blobs = img.find_blobs([BLACK_THRESHOLD],
                          pixels_threshold=MIN_PIXELS,
                          area_threshold=AREA_THRESHOLD,
                          x_stride=1,
                          y_stride=1,
                          margin=15,
                          )
    if blobs:
      valid_blobs = []
      for blob in blobs:
          #print(blob.density())
          if blob.density()<max_density and blob.density()>min_density :
              if blob.area()>min_area :#可调
                  img.draw_rectangle(blob.rect(), color=(255,0,0))
                  #print(blob.density(),blob.solidity(),blob.convexity())#像素数除以其边界框区域,使用最小区域旋转矩形与边界矩形来测量密度,对象的凸度
                  ##0.152698 0.167296 0.423312
                  # valid_blobs.append(blob)
                  if blob.solidity()<max_solidity:
                      if blob.convexity()<max_convexity:
                          valid_blobs.append(blob)
    if valid_blobs:
      for min_density_blob in valid_blobs:
          statistics=img.get_statistics(roi=min_density_blob.rect())#统计信息
          print(statistics)
          #print(min_density_blob.density(),min_density_blob.solidity(),min_density_blob.convexity())#0.1649351 0.1727304 0.4629062
          img.draw_line(min_density_blob.major_axis_line())
          img.draw_line(min_density_blob.minor_axis_line())
          intersection=line_intersection(min_density_blob.major_axis_line(), min_density_blob.minor_axis_line())
          duijiao_l1=(min_density_blob.min_corners()[0][0], min_density_blob.min_corners()[0][1],min_density_blob.min_corners()[2][0], min_density_blob.min_corners()[2][1])
          duijiao_l2=(min_density_blob.min_corners()[1][0], min_density_blob.min_corners()[1][1],min_density_blob.min_corners()[3][0], min_density_blob.min_corners()[3][1])
          intersection_2=line_intersection(duijiao_l1, duijiao_l2)


          lx1=abs(min_density_blob.major_axis_line()[0]-min_density_blob.major_axis_line()[2])
          ly1=abs(min_density_blob.major_axis_line()[1]-min_density_blob.major_axis_line()[3])
          l1=lx1**2+ly1**2
          l1=l1**(1/2)
          lx2=abs(min_density_blob.minor_axis_line()[0]-min_density_blob.minor_axis_line()[2])
          ly2=abs(min_density_blob.minor_axis_line()[1]-min_density_blob.minor_axis_line()[3])
          l2=lx2**2+ly2**2
          l2=l2**(1/2)
          print(l2/l1)

          if l2/l1>0.68:#可调
              print(statistics.mode())
              if statistics.mode()>95 :
                  flag=1
                  #img.draw_rectangle(min_density_blob.rect(), color=(255,0,0))
                  img.draw_cross(min_density_blob.min_corners()[0][0], min_density_blob.min_corners()[0][1])
                  img.draw_cross(min_density_blob.min_corners()[1][0], min_density_blob.min_corners()[1][1])
                  img.draw_cross(min_density_blob.min_corners()[2][0], min_density_blob.min_corners()[2][1])
                  img.draw_cross(min_density_blob.min_corners()[3][0], min_density_blob.min_corners()[3][1])
                  img.draw_circle(min_density_blob.enclosing_circle(),color=(255,255,0), thickness=2, fill=False)
                  # img.draw_cross(int(intersection[0]),int(intersection[1]), color=(255,0,0))
                  img.draw_cross(int(intersection_2[0]),int(intersection_2[1]), color=(255,0,255))
                  img.draw_cross(target_x,target_y, color=(0,0,0))
                  print(intersection)#靶心坐标
                  #串口发送示例
                  data = bytearray([
                      0x55,  # 起始符
                      (target_x >> 8) & 0xFF,  # X坐标高8位
                      target_x & 0xFF,         # X坐标低8位
                      (DISPLAY_HEIGHT - target_y) >> 8 & 0xFF,  # Y坐标高8位
                      (DISPLAY_HEIGHT - target_y) & 0xFF,       # Y坐标低8位
                      int(intersection[0]) >> 8 & 0xFF,         # 靶心X高8位
                      int(intersection[0]) & 0xFF,              # 靶心低8位
                      (DISPLAY_HEIGHT - int(intersection[1])) >> 8 & 0xFF,  # 靶心Y高8位
                      (DISPLAY_HEIGHT - int(intersection[1])) & 0xFF        # 靶心Y低8位
                  ])
                  uart.send(data)
              else:#如果没识别到
                      data = bytearray([0x55,0,0,0,0,0,0,0,0])
                      print(0x55,0,0,0,0,0,0,0,0)
                      uart.send(data)

          else:#如果没识别到
                  data = bytearray([0x55,0,0,0,0,0,0,0,0])
                  print(0x55,0,0,0,0,0,0,0,0)
                  uart.send(data)
    else:#如果没识别到
          data = bytearray([0x55,0,0,0,0,0,0,0,0])
          print(0x55,0,0,0,0,0,0,0,0)
          uart.send(data)
    return flag

def blob_2(img):
    print("执行中阈值识别")
    flag=0
    # 查找符合阈值的色块
    blobs = img.find_blobs([BLACK_THRESHOLD2],
                          pixels_threshold=MIN_PIXELS,
                          area_threshold=AREA_THRESHOLD,
                          x_stride=1,
                          y_stride=1,
                          margin=15,
                          )
    if blobs:
      valid_blobs = []
      for blob in blobs:
          #print(blob.density())
          if blob.density()<max_density and blob.density()>min_density :
              if blob.area()>min_area :#可调
                  img.draw_rectangle(blob.rect(), color=(255,0,0))
                  #print(blob.density(),blob.solidity(),blob.convexity())#像素数除以其边界框区域,使用最小区域旋转矩形与边界矩形来测量密度,对象的凸度
                  ##0.152698 0.167296 0.423312
                  # valid_blobs.append(blob)
                  if blob.solidity()<max_solidity:
                      if blob.convexity()<max_convexity:
                          valid_blobs.append(blob)
    if valid_blobs:
      for min_density_blob in valid_blobs:
          statistics=img.get_statistics(roi=min_density_blob.rect())#统计信息
          print(statistics)
          #print(min_density_blob.density(),min_density_blob.solidity(),min_density_blob.convexity())#0.1649351 0.1727304 0.4629062
          img.draw_line(min_density_blob.major_axis_line())
          img.draw_line(min_density_blob.minor_axis_line())
          intersection=line_intersection(min_density_blob.major_axis_line(), min_density_blob.minor_axis_line())
          duijiao_l1=(min_density_blob.min_corners()[0][0], min_density_blob.min_corners()[0][1],min_density_blob.min_corners()[2][0], min_density_blob.min_corners()[2][1])
          duijiao_l2=(min_density_blob.min_corners()[1][0], min_density_blob.min_corners()[1][1],min_density_blob.min_corners()[3][0], min_density_blob.min_corners()[3][1])
          intersection_2=line_intersection(duijiao_l1, duijiao_l2)


          lx1=abs(min_density_blob.major_axis_line()[0]-min_density_blob.major_axis_line()[2])
          ly1=abs(min_density_blob.major_axis_line()[1]-min_density_blob.major_axis_line()[3])
          l1=lx1**2+ly1**2
          l1=l1**(1/2)
          lx2=abs(min_density_blob.minor_axis_line()[0]-min_density_blob.minor_axis_line()[2])
          ly2=abs(min_density_blob.minor_axis_line()[1]-min_density_blob.minor_axis_line()[3])
          l2=lx2**2+ly2**2
          l2=l2**(1/2)
          print(l2/l1)
          if l2/l1>0.68:#可调
              if statistics.mode()>70 and statistics.median()>110:
                  flag=1
                  #img.draw_rectangle(min_density_blob.rect(), color=(255,0,0))
                  img.draw_cross(min_density_blob.min_corners()[0][0], min_density_blob.min_corners()[0][1])
                  img.draw_cross(min_density_blob.min_corners()[1][0], min_density_blob.min_corners()[1][1])
                  img.draw_cross(min_density_blob.min_corners()[2][0], min_density_blob.min_corners()[2][1])
                  img.draw_cross(min_density_blob.min_corners()[3][0], min_density_blob.min_corners()[3][1])
                  img.draw_circle(min_density_blob.enclosing_circle(),color=(255,255,0), thickness=2, fill=False)
                  # img.draw_cross(int(intersection[0]),int(intersection[1]), color=(255,0,0))
                  img.draw_cross(int(intersection_2[0]),int(intersection_2[1]), color=(255,0,255))
                  img.draw_cross(target_x,target_y, color=(0,0,0))
                  #print(intersection)#靶心坐标
                  #串口发送示例
                  data = bytearray([
                      0x55,  # 起始符
                      (target_x >> 8) & 0xFF,  # X坐标高8位
                      target_x & 0xFF,         # X坐标低8位
                      (DISPLAY_HEIGHT - target_y) >> 8 & 0xFF,  # Y坐标高8位
                      (DISPLAY_HEIGHT - target_y) & 0xFF,       # Y坐标低8位
                      int(intersection[0]) >> 8 & 0xFF,         # 靶心X高8位
                      int(intersection[0]) & 0xFF,              # 靶心低8位
                      (DISPLAY_HEIGHT - int(intersection[1])) >> 8 & 0xFF,  # 靶心Y高8位
                      (DISPLAY_HEIGHT - int(intersection[1])) & 0xFF        # 靶心Y低8位
                  ])
                  uart.send(data)
              else:#如果没识别到
                      data = bytearray([0x55,0,0,0,0,0,0,0,0])
                      print(0x55,0,0,0,0,0,0,0,0)
                      uart.send(data)

          else:#如果没识别到
                  data = bytearray([0x55,0,0,0,0,0,0,0,0])
                  print(0x55,0,0,0,0,0,0,0,0)
                  uart.send(data)
    else:#如果没识别到
          data = bytearray([0x55,0,0,0,0,0,0,0,0])
          print(0x55,0,0,0,0,0,0,0,0)
          uart.send(data)
    return flag
def blob_3(img):
    print("执行大阈值识别")
    flag=0
    # 查找符合阈值的色块
    blobs = img.find_blobs([BLACK_THRESHOLD3],
                          pixels_threshold=MIN_PIXELS,
                          area_threshold=AREA_THRESHOLD,
                          x_stride=1,
                          y_stride=1,
                          margin=15,
                          )
    if blobs:
      valid_blobs = []
      for blob in blobs:
          #print(blob.density())
          if blob.density()<max_density and blob.density()>min_density :
              if blob.area()>min_area :#可调
                  img.draw_rectangle(blob.rect(), color=(255,0,0))
                  #print(blob.density(),blob.solidity(),blob.convexity())#像素数除以其边界框区域,使用最小区域旋转矩形与边界矩形来测量密度,对象的凸度
                  ##0.152698 0.167296 0.423312
                  # valid_blobs.append(blob)
                  if blob.solidity()<max_solidity:
                      if blob.convexity()<max_convexity:
                          valid_blobs.append(blob)
    if valid_blobs:
      for min_density_blob in valid_blobs:
          statistics=img.get_statistics(roi=min_density_blob.rect())#统计信息
          print(statistics)
          #print(min_density_blob.density(),min_density_blob.solidity(),min_density_blob.convexity())#0.1649351 0.1727304 0.4629062
          img.draw_line(min_density_blob.major_axis_line())
          img.draw_line(min_density_blob.minor_axis_line())
          intersection=line_intersection(min_density_blob.major_axis_line(), min_density_blob.minor_axis_line())
          duijiao_l1=(min_density_blob.min_corners()[0][0], min_density_blob.min_corners()[0][1],min_density_blob.min_corners()[2][0], min_density_blob.min_corners()[2][1])
          duijiao_l2=(min_density_blob.min_corners()[1][0], min_density_blob.min_corners()[1][1],min_density_blob.min_corners()[3][0], min_density_blob.min_corners()[3][1])
          intersection_2=line_intersection(duijiao_l1, duijiao_l2)


          lx1=abs(min_density_blob.major_axis_line()[0]-min_density_blob.major_axis_line()[2])
          ly1=abs(min_density_blob.major_axis_line()[1]-min_density_blob.major_axis_line()[3])
          l1=lx1**2+ly1**2
          l1=l1**(1/2)
          lx2=abs(min_density_blob.minor_axis_line()[0]-min_density_blob.minor_axis_line()[2])
          ly2=abs(min_density_blob.minor_axis_line()[1]-min_density_blob.minor_axis_line()[3])
          l2=lx2**2+ly2**2
          l2=l2**(1/2)
          print(l2/l1)

          if l2/l1>0.68 and l2/l1<0.96:#可调
              if statistics.mode()>100 :
                  flag=1
                  #img.draw_rectangle(min_density_blob.rect(), color=(255,0,0))
                  img.draw_cross(min_density_blob.min_corners()[0][0], min_density_blob.min_corners()[0][1])
                  img.draw_cross(min_density_blob.min_corners()[1][0], min_density_blob.min_corners()[1][1])
                  img.draw_cross(min_density_blob.min_corners()[2][0], min_density_blob.min_corners()[2][1])
                  img.draw_cross(min_density_blob.min_corners()[3][0], min_density_blob.min_corners()[3][1])
                  img.draw_circle(min_density_blob.enclosing_circle(),color=(255,255,0), thickness=2, fill=False)
                  # img.draw_cross(int(intersection[0]),int(intersection[1]), color=(255,0,0))
                  img.draw_cross(int(intersection_2[0]),int(intersection_2[1]), color=(255,0,255))
                  img.draw_cross(target_x,target_y, color=(0,0,0))
                  #print(intersection)#靶心坐标
                  #串口发送示例
                  data = bytearray([
                      0x55,  # 起始符
                      (target_x >> 8) & 0xFF,  # X坐标高8位
                      target_x & 0xFF,         # X坐标低8位
                      (DISPLAY_HEIGHT - target_y) >> 8 & 0xFF,  # Y坐标高8位
                      (DISPLAY_HEIGHT - target_y) & 0xFF,       # Y坐标低8位
                      int(intersection[0]) >> 8 & 0xFF,         # 靶心X高8位
                      int(intersection[0]) & 0xFF,              # 靶心低8位
                      (DISPLAY_HEIGHT - int(intersection[1])) >> 8 & 0xFF,  # 靶心Y高8位
                      (DISPLAY_HEIGHT - int(intersection[1])) & 0xFF        # 靶心Y低8位
                  ])
                  uart.send(data)
              else:#如果没识别到
                      data = bytearray([0x55,0,0,0,0,0,0,0,0])
                      print(0x55,0,0,0,0,0,0,0,0)
                      uart.send(data)

          else:#如果没识别到
                  data = bytearray([0x55,0,0,0,0,0,0,0,0])
                  print(0x55,0,0,0,0,0,0,0,0)
                  uart.send(data)
    else:#如果没识别到
          data = bytearray([0x55,0,0,0,0,0,0,0,0])
          print(0x55,0,0,0,0,0,0,0,0)
          uart.send(data)
    return flag

def main():
    global flag1
    global flag2
    global flags
    try:
        # 初始化设备 / Initialize devices
        sensor = init_sensor()
        init_display()
        sensor.run()
        #uart.write(bytearray([0x55, 0xaa, 0xff, int(222)&0xFF, int(333)&0xFF, 0xfa]))
        clock = time.clock()

        while True:
            clock.tick()
            # 捕获图像 / Capture image
            img = sensor.snapshot()

            print("***************************************************************")

            flag1=blob_1(img)
            print(flag1)
            if flag1==0:
                clock.tick()
                # 捕获图像 / Capture image
                img = sensor.snapshot()
                flag2=blob_2(img)
                print(flag2)
                if flag2==0:#
                    pass




            # 显示FPS / Display FPS
            fps_text = f'FPS: {clock.fps():.3f}'
            img.draw_string_advanced(0, 0, 30, fps_text, color=(255, 255, 255))

            # 显示图像并打印FPS / Display image and print FPS
            Display.show_image(img)
            #print(clock.fps())

    except KeyboardInterrupt as e:
        print("用户中断 / User interrupted: ", e)
    except Exception as e:
        print(f"发生错误 / Error occurred: {e}")
    finally:
        # 清理资源 / Cleanup resources
        if 'sensor' in locals() and isinstance(sensor, Sensor):
            sensor.stop()
        Display.deinit()
        MediaManager.deinit()

if __name__ == "__main__":
    main()


















