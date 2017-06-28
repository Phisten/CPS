﻿using System;
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
        Gray hueInRangeLeft1 = new Gray(0d), hueInRangeRight1 = new Gray(0d);
        Gray hueInRangeLeft2 = new Gray(0d), hueInRangeRight2 = new Gray(0d);
        Gray hueInRangeLeft3 = new Gray(0d), hueInRangeRight3 = new Gray(0d);


        public Form1()
        {
            InitializeComponent();
            frame = new Image<Bgr, byte>(new Size(640, 480));
            btnHueInRange_Click(null, null);
        }
        private Capture cap = null;

        private void Form1_Load(object sender, EventArgs e)
        {
            //cap = new Capture(0);
            // Application.Idle += new EventHandler(Application_Idle);
        }


        private void button1_Click(object sender, EventArgs e)
        {
            cap = new Capture(0); //連結攝影機
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

            //pictureBox3.Image = HueCOI.ToBitmap();
            //根據目前要搜索的顏色二值化影像


            g = Graphics.FromImage(pictureBox2.Image);


            CvBlob maxBlob = ColorFillter(HueImg, hueInRangeLeft1, hueInRangeRight1);
            DrawRect(g, maxBlob);
            maxBlob = ColorFillter(HueImg, hueInRangeLeft2, hueInRangeRight2);
            DrawRect(g, maxBlob);
            maxBlob = ColorFillter(HueImg, hueInRangeLeft3, hueInRangeRight3);
            DrawRect(g, maxBlob);


        }

        private static void DrawRect(Graphics g, CvBlob maxBlob)
        {
            CvBlob curBlob = maxBlob;
            if (!(curBlob.BoundingBox.Width < 80 || curBlob.BoundingBox.Height < 100))
            {
                g.DrawRectangle(new Pen(Color.Yellow, 2.0f), maxBlob.BoundingBox);
            }
        }

        private CvBlob ColorFillter(Image<Gray, byte> HueImg, Gray hueInRangeLeft,Gray hueInRangeRight)
        {
            Image<Gray, byte> HueCOI = HueImg.InRange(hueInRangeLeft, hueInRangeRight);
            //Image<Gray, byte> erodeimage = HueCOI.Erode(1);
            //Image<Gray, byte> dilateimage = erodeimage.Dilate(1);


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
            return maxBlob;
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
            hueInRangeRight1 = new Gray(tmp);
            double.TryParse(tbHueRangeLeft.Text, out tmp);
            hueInRangeLeft1 = new Gray(tmp);

            double.TryParse(textBox2.Text, out tmp);
            hueInRangeRight2 = new Gray(tmp);
            double.TryParse(textBox1.Text, out tmp);
            hueInRangeLeft2 = new Gray(tmp);

            double.TryParse(textBox4.Text, out tmp);
            hueInRangeRight3 = new Gray(tmp);
            double.TryParse(textBox3.Text, out tmp);
            hueInRangeLeft3 = new Gray(tmp);
        }

    }
}
