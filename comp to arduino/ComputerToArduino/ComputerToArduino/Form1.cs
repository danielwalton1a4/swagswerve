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
using System.Timers;
using System.Windows.Forms;
using MathNet.Numerics.LinearAlgebra;
using MathNet.Numerics.LinearAlgebra.Double;



namespace ComputerToArduino
{
    public partial class Form1 : Form
    {
        bool isConnected = false;
        String[] ports;
        SerialPort port;
        String receivedData = "";

        double LEFT_STICK_DEADZONE = 0.1;   //deadzones for our sticks
        double RIGHT_STICK_DEADZONE = 0.1;
        double ZOOM_ZOOM = 100;             //this is a constant that multiplies how far we want to go on each time step. bigger number makes robot go faster.
        double FRAME_WIDTH = 0.446;         //depth and width of the robot frame in meters
        double FRAME_DEPTH = 0.446;


        float rotAmount = 0;        // used for animating the rotating doohickey in the GUI

        bool reset = false;             //tied to a button on the controller. set this to 1 when you wat to manually change field alignment

        double leftStickX = 0;      //xy coords of sticks
        double rightStickX = 0;
        double leftStickY = 0;
        double rightStickY = 0;

        double leftStickAngle = 0;  //angle of left stick
        double rightStickAngle = 0; //angle of right stick
        double leftStickMag = 0;    //magnitude of left stick
        double rightStickMag = 0;   //magnitude of right stick
        double yaw = 0;             //yaw from IMU
        double yawZero = 0;         //zero direction for the bot

        private static System.Timers.Timer JoystickTimer;

        Matrix<double> m = Matrix<double>.Build.Random(2, 2);

        public Form1()
        {
            TimerInit();
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

        private  void TimerInit()
        {
            JoystickTimer = new System.Timers.Timer(50);
            JoystickTimer.Elapsed += OnTimedEvent;
            JoystickTimer.AutoReset = true;
            JoystickTimer.Enabled = true;
        }

        private void OnTimedEvent(Object source, ElapsedEventArgs e)
        {
            if (checkBox1.Checked)
            {
                SendMessage("J1X", (trackBar1.Value * 0.2).ToString());
                SendMessage("J1Y", (trackBar2.Value * 0.2).ToString());
            }
            if (checkBox2.Checked)
            {
                SendMessage("J2X", (trackBar3.Value * 0.2).ToString());
                SendMessage("J2Y", (trackBar4.Value * 0.2).ToString());
            }
            leftStickMag = MAGNITUDE(leftStickX, leftStickY);
            rightStickMag = MAGNITUDE(rightStickX, rightStickY);
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
            textBox1.Enabled = true;
            groupBox3.Enabled = true;

        }

        private void disableControls()
        {
            textBox1.Enabled = false;
            groupBox3.Enabled = false;
        }

        private void resetDefaults()
        {
            textBox1.Text = "";
            
        }

        private void button3_Click(object sender, EventArgs e)
        {
            SendMessage("RST", "1");
            WriteToDebugBox("Resetting IMU...\n");
            Debug.Write("Restting IMU...\n");
        }

        public double MAGNITUDE(double x, double y)
        {
            return Math.Sqrt(x * x + y * y);
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

            System.Drawing.Drawing2D.Matrix mtrx = new System.Drawing.Drawing2D.Matrix();

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
                textBox1.Text = message;
                receivedData = "";
                switch (GetID(message)) {
                    case "YAW":
                        Bitmap frameVisual = new Bitmap(pictureBox2.Image);
                        try
                        {
                            yaw = double.Parse(GetPayload(message));
                            pictureBox1.Image = RotateImg(frameVisual, (float) yaw);
                        } catch
                        {
                            Debug.Write("Failed to parse command: ");
                            Debug.WriteLine(message);
                            Debug.Write("Payload was: ");
                            Debug.WriteLine(GetPayload(message));
                            WriteToDebugBox("Failed to parse command:" + message + "\nPayload was: " + GetPayload(message) + "\n");
                        }
                        frameVisual.Dispose();
                        break;
                    case "DBG":
                        WriteToDebugBox(GetPayload(message));
                        break;

                }
                //if (message.Length < 10) {
                //    message.Remove(message.Length - 1);
                //    rotAmount = float.Parse(GetPayload(message));
                //    Bitmap frameVisual = new Bitmap(pictureBox2.Image);
                //    pictureBox1.Image = RotateImg(frameVisual, rotAmount);
                //    frameVisual.Dispose();
                //}
            } else
            {
                receivedData += tempStr;
            }
            
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
             * EN1 - module 1 encoder reading
             * EN2 - module 2 encoder reading
             * EN3 - module 3 encoder reading
             * EN4 - module 4 encoder reading
             * DBG - debug message (string)
             * 
             * M1A - module 1 angle command
             * M1D - module 1 drive command
             * M2A - module 2 angle command
             * M2D - module 2 drive command
             * M3A - module 3 angle command
             * M3D - module 3 drive command
             * M4A - module 4 angle command
             * M4D - module 4 drive command
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
            if (str[0] != '#' || str[str.Length - 1] != '\n' || str.Length > 256)
            {
                //if the message does not start and end correctly then the message is invalid and we toss it.
                //it is technically possible that we might want to send a payload that happens to exactly match
                //the binary representation of "INVALID_MESSAGE" but like, too bad. you dont get to do that now.
                return "INVALID_MESSAGE";
            }
            else
            {

                return str.Substring(4, str.Length - 4);
            }
        }

        private void WriteToDebugBox(String msg)
        {
            Debug.Print(msg);
            if (textBox2.Text.Length > 300)
            {
                textBox2.Text = "";
            }
            textBox2.Text += "\n" + msg;
        }

        private void textBox3_TextChanged(object sender, EventArgs e)
        {
            port.Write(textBox3.Text);
        }

        private Vector<double> Rotate2(Vector<double> v, double theta) {
            // takes in a 2x1 vector and angle (in degrees), returns that vector rotated by the angle
            double toDeg = Math.PI / 180 * theta;
            Matrix<double> rotMat = DenseMatrix.OfArray(new double[,] {
                {Math.Cos(toDeg), -Math.Sin(toDeg)},
                {Math.Sin(toDeg), Math.Cos(toDeg)}
            });
            return rotMat.Multiply(v);
        }
    }
}
