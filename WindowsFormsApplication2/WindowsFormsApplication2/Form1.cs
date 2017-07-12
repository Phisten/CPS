using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Emgu.CV;
using Emgu.CV.Structure;
using Emgu.Util;
using Emgu.CV.UI;
using Emgu.CV.Cvb;



namespace WindowsFormsApplication2
{
    public partial class Form1 : Form
    {
        Image<Bgr, Byte> frame;
        Gray hueInRangeLeft = new Gray(0d), hueInRangeRight = new Gray(0d);
        int angleRowDataCount = 10;
        List<double> angleRowData;

        public Form1()
        {
            InitializeComponent();
            frame = new Image<Bgr, byte>(new Size(640, 480));

            angleRowData = new List<double>(angleRowDataCount);
            for (int i = 0; i < angleRowDataCount; i++)
                angleRowData.Add(0);
        }
        private Capture cap = null;

        private void Form1_Load(object sender, EventArgs e)
        {
            //cap = new Capture(0);
            // Application.Idle += new EventHandler(Application_Idle);
        }


        private void button1_Click(object sender, EventArgs e)
        {
            cap = new Capture(1); //連結攝影機
            Application.Idle += new EventHandler(Application_Idle);//在idle的event下，把畫面設定到pictureBox1上

        }

        private void button2_Click(object sender, EventArgs e)
        {

            Application.Idle -= new EventHandler(Application_Idle);//清除idle的值
            pictureBox1.Image = null;//清除pictureBox1的圖

        }

        private void button3_Click(object sender, EventArgs e)
        {

        }
        void Application_Idle(object sender, EventArgs e)
        {
            //由相機載入影像
            frame = cap.QueryFrame();
            Bitmap b1 = frame.ToBitmap();
            Graphics g = Graphics.FromImage(b1);
            g.DrawRectangle(new Pen(Color.Red, 2.0f), SrcImgRoiRect);
            pictureBox1.Image = b1;

            //根據ROI(相機可能會拍到色卡的區域) 切割影像
            frame.ROI = SrcImgRoiRect;
            Image<Hsv, byte> roiimg = frame.Convert<Hsv, byte>();
            Image<Gray, byte> HueImg = roiimg[0]; //H
            pictureBox4.Image = roiimg[1].ToBitmap(); //S
            pictureBox5.Image = roiimg[2].ToBitmap(); //V
            if (chkConvertHSV.Checked)
            {
                pictureBox2.Image = HueImg.Convert<Hsv, byte>().ToBitmap();
            }
            else
            {
                pictureBox2.Image = roiimg.ToBitmap();
            }
            
            g = Graphics.FromImage(pictureBox2.Image);
            g.DrawRectangle(new Pen(Color.Red, 2.0f), HueImgRoiRect);

            //根據目前要搜索的顏色二值化影像
            Image<Gray, byte> HueCOI = HueImg.InRange(hueInRangeLeft, hueInRangeRight);
            //Image<Gray, byte> erodeimage = HueCOI.Erode(1);
            //Image<Gray, byte> dilateimage = erodeimage.Dilate(1);
            pictureBox3.Image = HueCOI.ToBitmap();

            
            //根據COI二值化影像找出最大色塊
            CvBlob maxBlob = null;
            CvBlobDetector m_blobDetector = new CvBlobDetector();
            CvBlobs blobs = new CvBlobs();
            m_blobDetector.Detect(HueCOI, blobs);
            if (blobs.Count > 0)
            {
                maxBlob = blobs.ElementAt(0).Value;
                foreach (var item in blobs)
                {
                    if (item.Value.Area > maxBlob.Area)
                    {
                        maxBlob = item.Value;
                    }
                }
            }
            
            if (maxBlob != null )
            {
                Rectangle tmpRect = maxBlob.BoundingBox;
                int whiteWidth = tmpRect.Width / 8;
                int whiteHeight = tmpRect.Height / 8;

                tmpRect.X = Math.Max(0, tmpRect.X - whiteWidth);
                tmpRect.Y = Math.Max(0, tmpRect.Y - whiteHeight);
                tmpRect.Width = Math.Min(roiimg.Width - tmpRect.X, tmpRect.Width + whiteWidth * 2);
                tmpRect.Height = Math.Min(roiimg.Height - tmpRect.Y, tmpRect.Height + whiteHeight * 2);
                roiimg.ROI = tmpRect;
                
                Image<Hsv, byte> targetROI = roiimg.Copy();

                pictureBox6.Image = targetROI.ToBitmap() ;


                //angle and dist
                Image<Gray, byte> targetThres = targetROI.Convert<Gray, byte>();
                targetThres = targetThres.ThresholdBinary(new Gray(120), new Gray(255));
                //TODO : AUTO THRESHOLD
                //targetThres = targetThres.ThresholdAdaptive(new Gray(255),Emgu.CV.CvEnum.ADAPTIVE_THRESHOLD_TYPE.CV_ADAPTIVE_THRESH_GAUSSIAN_C,Emgu.CV.CvEnum.THRESH.CV_THRESH_OTSU,3,new Gray(0));

                //get boundingBox cal width / height

                int targetCenterX = -1;
                int targetCenterY = -1;
                int targetWidth = -1;
                int targetHeight = -1;
                double targetAngle = 0.0d;
                double targetDist = 0.0d;
                {
                    CvBlob targetBlob = null;
                    CvBlobDetector m_targetblobDetector = new CvBlobDetector();
                    CvBlobs targetblobs = new CvBlobs();
                    m_blobDetector.Detect(targetThres, targetblobs);
                    if (targetblobs.Count > 0)
                    {
                        targetBlob = targetblobs.ElementAt(0).Value;
                        foreach (var item in targetblobs)
                        {
                            if (item.Value.Area > targetBlob.Area)
                            {
                                targetBlob = item.Value;
                            }
                        }
                    }
                    if (targetBlob != null)
                    {
                        targetCenterX = targetBlob.BoundingBox.X + targetBlob.BoundingBox.Width / 2;
                        targetCenterY = targetBlob.BoundingBox.Y + targetBlob.BoundingBox.Height / 2;
                        targetWidth = targetBlob.BoundingBox.Width;
                        targetHeight = targetBlob.BoundingBox.Height;
                        targetAngle = Math.Max(Math.Round( ((double)targetHeight / (double)targetWidth-1d) * 136d),0d);

                        int centerHeight = 0;

                        //角度計算
                        int targetX = targetBlob.BoundingBox.X;
                        int targetY = targetBlob.BoundingBox.Y;
                        //boundingBox sum for angle +- check
                        //int[] targetHorizontalSum = new int[targetWidth];
                        int leftWeights=0, rightWeights=0;
                        for (int i = targetX; i < targetX+targetWidth; i++)
                        {
                            for (int j = targetY; j < targetY+targetHeight; j++)
                            {
                                if (targetThres[j,  i].Intensity == 0)
                                {
                                    //angle param
                                    if (i < targetCenterX)
                                        leftWeights++;
                                    else
                                        rightWeights++;

                                    //dist param
                                    if (i == targetCenterX)
                                        centerHeight++;
                                }
                            }
                        }
                        if (rightWeights > leftWeights)
                        {
                            targetAngle *= -1;
                        }
                        //角度均值
                        angleRowData.RemoveAt(0);
                        angleRowData.Add(targetAngle);
                        targetAngle = Math.Round(angleRowData.Average());

                        //距離計算
                        //TODO: config
                        double baseDist1 = 40;
                        double basePixelHeight1 = 285;
                        double baseDist2 = 150;
                        double basePixelHeight2 = 77;
                        targetDist = Math.Round(baseDist2 * basePixelHeight2 / centerHeight);


                    }
                }

                if (targetHeight == 0)
                    targetHeight = -1;
                label1.Text = targetWidth + "," + targetHeight + " --> " + Math.Round((double)targetWidth / (double)targetHeight, 2);
                label2.Text = "角度:" + targetAngle;
                label3.Text = "距離:" + targetDist;
                pictureBox7.Image = targetThres.ToBitmap();
            }




        }


        Point SrcImgRoiP1 = new Point(0, 0), SrcImgRoiP2 = new Point(0, 0);
        Rectangle SrcImgRoiRect = new Rectangle(0, 0, 0, 0);
        bool setSrcImgRoiRect = false;
        private void pictureBox1_MouseDown(object sender, MouseEventArgs e)
        {
            setSrcImgRoiRect = true;
            SrcImgRoiP1.X = e.X;
            SrcImgRoiP1.Y = e.Y;
            SrcImgRoiP2.X = e.X;
            SrcImgRoiP2.Y = e.Y;
        }

        private void pictureBox1_MouseUp(object sender, MouseEventArgs e)
        {
            if (setSrcImgRoiRect)
            {
                SrcImgRoiP2.X = e.X;
                SrcImgRoiP2.Y = e.Y;
                if (SrcImgRoiP2.X > SrcImgRoiP1.X && SrcImgRoiP2.Y > SrcImgRoiP1.Y)
                {
                    SrcImgRoiRect.X = SrcImgRoiP1.X;
                    SrcImgRoiRect.Y = SrcImgRoiP1.Y;
                    SrcImgRoiRect.Width = SrcImgRoiP2.X - SrcImgRoiP1.X;
                    SrcImgRoiRect.Height = SrcImgRoiP2.Y - SrcImgRoiP1.Y;
                }
                else if (SrcImgRoiP2.X > SrcImgRoiP1.X && SrcImgRoiP2.Y < SrcImgRoiP1.Y)
                {
                    SrcImgRoiRect.X = SrcImgRoiP1.X;
                    SrcImgRoiRect.Y = SrcImgRoiP2.Y;
                    SrcImgRoiRect.Width = SrcImgRoiP2.X - SrcImgRoiP1.X;
                    SrcImgRoiRect.Height = SrcImgRoiP1.Y - SrcImgRoiP2.Y;
                }
                else if (SrcImgRoiP2.X < SrcImgRoiP1.X && SrcImgRoiP2.Y > SrcImgRoiP1.Y)
                {
                    SrcImgRoiRect.X = SrcImgRoiP2.X;
                    SrcImgRoiRect.Y = SrcImgRoiP1.Y;
                    SrcImgRoiRect.Width = SrcImgRoiP1.X - SrcImgRoiP2.X;
                    SrcImgRoiRect.Height = SrcImgRoiP2.Y - SrcImgRoiP1.Y;
                }
                else if (SrcImgRoiP2.X < SrcImgRoiP1.X && SrcImgRoiP2.Y < SrcImgRoiP1.Y)
                {
                    SrcImgRoiRect.X = SrcImgRoiP2.X;
                    SrcImgRoiRect.Y = SrcImgRoiP2.Y;
                    SrcImgRoiRect.Width = SrcImgRoiP1.X - SrcImgRoiP2.X;
                    SrcImgRoiRect.Height = SrcImgRoiP1.Y - SrcImgRoiP2.Y;
                }
            }
            setSrcImgRoiRect = false;

        }
        private void pictureBox1_MouseMove(object sender, MouseEventArgs e)
        {

        }
        Point HueImgRoiP1 = new Point(0, 0), HueImgRoiP2 = new Point(0, 0);
        Rectangle HueImgRoiRect = new Rectangle(0, 0, 0, 0);
        bool setHueImgRoiRect = false;
        private void pictureBox2_MouseDown(object sender, MouseEventArgs e)
        {
            setHueImgRoiRect = true;
            HueImgRoiP1.X = e.X;
            HueImgRoiP1.Y = e.Y;
            HueImgRoiP2.X = e.X;
            HueImgRoiP2.Y = e.Y;
        }

        private void pictureBox2_MouseMove(object sender, MouseEventArgs e)
        {

        }

        private void pictureBox2_MouseUp(object sender, MouseEventArgs e)
        {
            if (setHueImgRoiRect)
            {
                HueImgRoiP2.X = e.X;
                HueImgRoiP2.Y = e.Y;
                if (HueImgRoiP2.X > HueImgRoiP1.X && HueImgRoiP2.Y > HueImgRoiP1.Y)
                {
                    HueImgRoiRect.X = HueImgRoiP1.X;
                    HueImgRoiRect.Y = HueImgRoiP1.Y;
                    HueImgRoiRect.Width = HueImgRoiP2.X - HueImgRoiP1.X;
                    HueImgRoiRect.Height = HueImgRoiP2.Y - HueImgRoiP1.Y;
                }
                else if (HueImgRoiP2.X > HueImgRoiP1.X && HueImgRoiP2.Y < HueImgRoiP1.Y)
                {
                    HueImgRoiRect.X = HueImgRoiP1.X;
                    HueImgRoiRect.Y = HueImgRoiP2.Y;
                    HueImgRoiRect.Width = HueImgRoiP2.X - HueImgRoiP1.X;
                    HueImgRoiRect.Height = HueImgRoiP1.Y - HueImgRoiP2.Y;
                }
                else if (HueImgRoiP2.X < HueImgRoiP1.X && HueImgRoiP2.Y > HueImgRoiP1.Y)
                {
                    HueImgRoiRect.X = HueImgRoiP2.X;
                    HueImgRoiRect.Y = HueImgRoiP1.Y;
                    HueImgRoiRect.Width = HueImgRoiP1.X - HueImgRoiP2.X;
                    HueImgRoiRect.Height = HueImgRoiP2.Y - HueImgRoiP1.Y;
                }
                else if (HueImgRoiP2.X < HueImgRoiP1.X && HueImgRoiP2.Y < HueImgRoiP1.Y)
                {
                    HueImgRoiRect.X = HueImgRoiP2.X;
                    HueImgRoiRect.Y = HueImgRoiP2.Y;
                    HueImgRoiRect.Width = HueImgRoiP1.X - HueImgRoiP2.X;
                    HueImgRoiRect.Height = HueImgRoiP1.Y - HueImgRoiP2.Y;
                }
            }
            setHueImgRoiRect = false;

            DenseHistogram hueHist = new DenseHistogram(256, new RangeF(0, 255));
            //參數一是要計算的顏色資料，這邊分割通道並取得Blue通道的顏色;依序[1]:Green->[2]:Red
            hueHist.Calculate(new IImage[] { new Image<Gray, byte>((Bitmap)pictureBox2.Image) }, false, null);
            HistogramViewer.Show(hueHist, "hue channel");

        }

        private void btnHueInRange_MouseClick(object sender, MouseEventArgs e)
        {

        }

        private void btnHueInRange_Click(object sender, EventArgs e)
        {
            double tmp;
            double.TryParse(tbHueRangeRight.Text, out tmp);
            hueInRangeRight = new Gray(tmp);
            double.TryParse(tbHueRangeLeft.Text, out tmp);
            hueInRangeLeft = new Gray(tmp);

        }

        private void tbHueRangeRight_TextChanged(object sender, EventArgs e)
        {

        }

    }
}
