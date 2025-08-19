using ComputerToArduino.Properties;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO;
using System.IO.Ports;
using System.Net.NetworkInformation;
using System.Reflection;
using System.Runtime;
using System.Text;
using System.Windows.Forms;

namespace ComputerToArduino
{
    public partial class Form1 : Form
    {
        bool isConnected = false;
        String[] ports;
        SerialPort port;
        String receivedData = "";
        float rotAmount = 0;
        //string folder = Path.GetDirectoryName(Assembly.GetExecutingAssembly().CodeBase);
        //string fullPath = Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().CodeBase), "\\frame_visual.png");
        //Bitmap frameVisual = new Bitmap(Path.Combine(Path.GetDirectoryName(Assembly.GetExecutingAssembly().CodeBase), "\\frame_visual.png"));

        public Form1()
        {
            
            CheckForIllegalCrossThreadCalls = false;
            InitializeComponent();
            disableControls();
            getAvailableComPorts();

            foreach (string port in ports)
            {
                comboBox1.Items.Add(port);
                Console.WriteLine(port);
                if (ports[0] != null)
                {
                    comboBox1.SelectedItem = ports[0];
                }
            }
            
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (!isConnected)
            {
                connectToArduino();
            } else
            {
                disconnectFromArduino();
            }
        }

        void getAvailableComPorts()
        {
            ports = SerialPort.GetPortNames();
        }

        private void connectToArduino()
        {
            isConnected = true;
            string selectedPort = comboBox1.GetItemText(comboBox1.SelectedItem);
            port = new SerialPort(selectedPort, 115200, Parity.None, 8, StopBits.One);
            port.DataReceived += new SerialDataReceivedEventHandler(port_DataReceived);
            port.Open();
            port.Write("#STAR\n");
            button1.Text = "Disconnect";
            enableControls();
        }

        private void Led1CheckboxClicked(object sender, EventArgs e)

        {
            if(isConnected)
            {
                if(checkBox1.Checked)
                {
                    port.Write("#LED1ON\n");
                }else
                {
                    port.Write("#LED1OF\n");
                }
            }
        }

        private void Led2CheckboxClicked(object sender, EventArgs e)

        {
            if (isConnected)
            {
                if (checkBox2.Checked)
                {
                    port.Write("#LED2ON\n");
                }
                else
                {
                    port.Write("#LED2OF\n");
                }
            }
        }

        private void Led3CheckboxClicked(object sender, EventArgs e)

        {
            if (isConnected)
            {
                if (checkBox3.Checked)
                {
                    port.Write("#LED3ON\n");
                }
                else
                {
                    port.Write("#LED3OF\n");
                }
            }
        }

        private void disconnectFromArduino()
        {
            isConnected = false;
            port.Write("#STOP\n");
            port.Close();
            button1.Text = "Connect";
            disableControls();
            resetDefaults();
        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (isConnected)
            {
                port.Write("#" + textBox1.Text + "\n");
            }
        }

        private void enableControls()
        {
            checkBox1.Enabled = true;
            checkBox2.Enabled = true;
            checkBox3.Enabled = true;
            button2.Enabled = true;
            textBox1.Enabled = true;
            groupBox1.Enabled = true;
            groupBox3.Enabled = true;

        }

        private void disableControls()
        {
            checkBox1.Enabled = false;
            checkBox2.Enabled = false;
            checkBox3.Enabled = false;
            button2.Enabled = false;
            textBox1.Enabled = false;
            groupBox1.Enabled = false;
            groupBox3.Enabled = false;
        }

        private void resetDefaults()
        {
            checkBox1.Checked = false;
            checkBox2.Checked = false;
            checkBox3.Checked = false;
            textBox1.Text = "";
            
        }

        private void groupBox3_Enter(object sender, EventArgs e)
        {

        }

        private void button3_Click(object sender, EventArgs e)
        {
            port.Write("#encoder\n");
            Debug.WriteLine("testing");
        }


        public static Bitmap RotateImg(Bitmap bmp, float angle)

        {

            int w = bmp.Width;

            int h = bmp.Height;

            Bitmap tempImg = new Bitmap(w, h);

            Graphics g = Graphics.FromImage(tempImg);

            g.DrawImageUnscaled(bmp, 1, 1);

            g.Dispose();

            GraphicsPath path = new GraphicsPath();

            path.AddRectangle(new RectangleF(0f, 0f, w, h));

            Matrix mtrx = new Matrix();

            mtrx.Rotate(angle);

            RectangleF rct = path.GetBounds(mtrx);

            Bitmap newImg = new Bitmap(Convert.ToInt32(rct.Width), Convert.ToInt32(rct.Height));

            g = Graphics.FromImage(newImg);

            g.TranslateTransform(-rct.X, -rct.Y);

            g.RotateTransform(angle);

            g.InterpolationMode = InterpolationMode.HighQualityBilinear;

            g.DrawImageUnscaled(tempImg, 0, 0);

            g.Dispose();

            tempImg.Dispose();

            return newImg;

        }


        private void port_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            // Show all the incoming data in the port's buffer

            string tempStr = port.ReadExisting();
            string message = "";
            if (tempStr[tempStr.Length - 1] == '\n')
            {
                message = receivedData + tempStr;
                Debug.WriteLine(message);
                Debug.WriteLine(message.Length);
                textBox1.Text = message;
                receivedData = "";
                switch (GetID(message)) {
                    case "YAW":
                        Bitmap frameVisual = new Bitmap(pictureBox2.Image);
                        pictureBox1.Image = RotateImg(frameVisual, float.Parse(GetPayload(message)));
                        frameVisual.Dispose();
                        break;
                    case "DBG":
                        Debug.Print(GetPayload(message));
                        if(textBox2.Text.Length > 1024) {
                            textBox2.Text.Remove(0, message.Length);
                        }
                        textBox2.Text += GetPayload(message);
                        break;

                }
                if (message.Length < 10) {
                    message.Remove(message.Length - 1);
                    rotAmount = float.Parse(message);
                    Bitmap frameVisual = new Bitmap(pictureBox2.Image);
                    pictureBox1.Image = RotateImg(frameVisual, rotAmount);
                    frameVisual.Dispose();
                }
            } else
            {
                receivedData += tempStr;
            }
            
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        public String GetID(String str)
        {
            /* Serial Comms Protocol:
             * # XXX YYY...Y \n
             * All messages start with #
             * XXX is the identifier. Always 3 chars
             * YYYYY is the payload. total message must be under 256 bytes but otherwise may be any length
             * All messages end with \n
             * All messages are sent in string format, so things that say they are a float are acrually just a string that you have to
             *  convert to a float
             * Example message: "#YAW21.28\n"
             *      this is a message from the arduino to the computer saying the yaw value is 21.28 degrees.
             * 
             * Valid IDs:
             * J1X - joystick 1 x value (float)
             * J1Y - joystick 1 y value (float)
             * J2X - joystick 2 x value (float)
             * J2Y - joystick 2 y value (float)
             * RST - reset button value (bool)
             * ACK - acknowledgement (int)
             * YAW - yaw value from the encoder in degrees (float)
             * DBG - debug message (string)
             * 
             */
            if (str[0] != '#' || str[str.Length - 1] != '\n' || str.Length > 256)
            {
                //if the message does not start and end correctly then the message is invalid and we toss it.
                return "INV";
            } else
            {
                return str.Substring(1, 3);
            }
        }

        public bool SendMessage(String ID, String payload) {
            port.Write("#" + ID + payload + "\n");
            return true;
        }

        public String GetPayload(String str)
        {
            /* Serial Comms Protocol:
             * # XXX YYY...Y \n
             * All messages start with #
             * XXX is the identifier. Always 3 chars
             * YYYYY is the payload. total message must b under 256 bytes, but otherwise can be any length
             * All messages end with \n
             */
            if (str[0] != '#' || str[str.Length - 1] != '\n' || str.Length > 256)
            {
                //if the message does not start and end correctly then the message is invalid and we toss it.
                //it is technically possible that we might want to send a payload that happens to exactly match
                //the binary representation of "INVALID_MESSAGE" but like, too bad. you dont get to do that now.
                return "INVALID_MESSAGE";
            }
            else
            {
                return str.Substring(4, str.Length - 2);
            }
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {

        }

        private void textBox2_TextChanged(object sender, EventArgs e) {

        }
    }
}
