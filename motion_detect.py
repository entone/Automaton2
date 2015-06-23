import cv2.cv as cv
import cv2
import urllib
import numpy as np
import gevent
import time

#URL = 'http://dashboard:1234@entropealabs.net:8181/MJPEG.CGI'
URL = 'http://entropealabs.net/camera/front.jpg'

last_image = None

class Target:

    def __init__(self, url):
        self.url = url
        self.last_image = None
        cv.NamedWindow("Target", 1)

    def get_image(self):
        stream=urllib.urlopen(self.url)
        bytes=''
        while True:
            bytes+=stream.read(1024)
            a = bytes.find('\xff\xd8')
            b = bytes.find('\xff\xd9')
            if a!=-1 and b!=-1:
                jpg = bytes[a:b+2]
                bytes= bytes[b+2:]
                source = cv2.imdecode(np.fromstring(jpg, dtype=np.uint8),cv2.CV_LOAD_IMAGE_COLOR)
                bitmap = cv.CreateImageHeader((source.shape[1], source.shape[0]), cv.IPL_DEPTH_8U, 3)
                cv.SetData(bitmap, source.tostring(), source.dtype.itemsize * 3 * source.shape[1])
                return bitmap

    def get_image2(self):
        try:
            resp = urllib.urlopen(self.url)
            jpg = np.asarray(bytearray(resp.read()), dtype="uint8")
            source = cv2.imdecode(np.fromstring(jpg, dtype=np.uint8),cv2.CV_LOAD_IMAGE_COLOR)
            bitmap = cv.CreateImageHeader((source.shape[1], source.shape[0]), cv.IPL_DEPTH_8U, 3)
            cv.SetData(bitmap, source.tostring(), source.dtype.itemsize * 3 * source.shape[1])
            self.last_image = bitmap
        except:
            bitmap = self.last_image
        return bitmap


    def run(self):
        # Capture first frame to get size
        frame = self.get_image2()
        frame_size = cv.GetSize(frame)
        color_image = cv.CreateImage(frame_size, 8, 3)
        grey_image = cv.CreateImage(frame_size, cv.IPL_DEPTH_8U, 1)
        moving_average = cv.CreateImage(frame_size, cv.IPL_DEPTH_32F, 3)

        first = True

        while True:
            closest_to_left = cv.GetSize(frame)[0]
            closest_to_right = cv.GetSize(frame)[1]
            print "getting Image"
            color_image = self.get_image2()
            print "got image"

            # Smooth to get rid of false positives
            cv.Smooth(color_image, color_image, cv.CV_GAUSSIAN, 3, 0)

            if first:
                difference = cv.CloneImage(color_image)
                temp = cv.CloneImage(color_image)
                cv.ConvertScale(color_image, moving_average, 1.0, 0.0)
                first = False
            else:
                cv.RunningAvg(color_image, moving_average, 0.30, None)

            # Convert the scale of the moving average.
            cv.ConvertScale(moving_average, temp, 1.0, 0.0)

            # Minus the current frame from the moving average.
            cv.AbsDiff(color_image, temp, difference)

            # Convert the image to grayscale.
            cv.CvtColor(difference, grey_image, cv.CV_RGB2GRAY)

            # Convert the image to black and white.
            cv.Threshold(grey_image, grey_image, 70, 255, cv.CV_THRESH_BINARY)

            # Dilate and erode to get people blobs
            cv.Dilate(grey_image, grey_image, None, 18)
            cv.Erode(grey_image, grey_image, None, 10)

            storage = cv.CreateMemStorage(0)
            contour = cv.FindContours(grey_image, storage, cv.CV_RETR_CCOMP, cv.CV_CHAIN_APPROX_SIMPLE)
            points = []

            while contour:
                bound_rect = cv.BoundingRect(list(contour))
                contour = contour.h_next()

                pt1 = (bound_rect[0], bound_rect[1])
                pt2 = (bound_rect[0] + bound_rect[2], bound_rect[1] + bound_rect[3])
                points.append(pt1)
                points.append(pt2)
                cv.Rectangle(color_image, pt1, pt2, cv.CV_RGB(255,0,0), 1)

            if len(points):
                center_point = reduce(lambda a, b: ((a[0] + b[0]) / 2, (a[1] + b[1]) / 2), points)
                cv.Circle(color_image, center_point, 40, cv.CV_RGB(255, 255, 255), 1)
                cv.Circle(color_image, center_point, 30, cv.CV_RGB(255, 100, 0), 1)
                cv.Circle(color_image, center_point, 20, cv.CV_RGB(255, 255, 255), 1)
                cv.Circle(color_image, center_point, 10, cv.CV_RGB(255, 100, 0), 1)

            cv.ShowImage("Target", color_image)

            # Listen for ESC key
            c = cv.WaitKey(7) % 0x100
            if c == 27:
                break

if __name__=="__main__":
    t = Target(URL)
    t.run()
