using MathNet.Numerics.LinearAlgebra;
using MathNet.Numerics.LinearAlgebra.Double;
using System;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.IO.Ports;
using System.Numerics;
using System.Timers;
using System.Windows.Forms;

namespace ComputerToArduino {
    public partial class Form1 : Form {
        bool isConnected = false;
        String[] ports;
        SerialPort port;
        String receivedData = "";

        double LEFT_STICK_DEADZONE = 0.1;   //deadzones for our sticks
        double RIGHT_STICK_DEADZONE = 0.1;
        double ZOOM_ZOOM = 10;             //this is a constant that multiplies how far we want to go on each time step. bigger number makes robot go faster.
        double FRAME_WIDTH = 0.446;         //depth and width of the robot frame in meters
        double FRAME_DEPTH = 0.446;


        float rotAmount = 0;        // used for animating the rotating doohickey in the GUI

        bool reset = false;             //tied to a button on the controller. set this to 1 when you wat to manually change field alignment

        double steerStickX = 0;      //xy coords of sticks
        double driveStickX = 0;
        double steerStickY = 0;
        double driveStickY = 0;

        double steerStickAngle = 0;  //angle of left stick
        double driveStickAngle = 0; //angle of right stick
        double steerStickMag = 0;    //magnitude of left stick
        double driveStickMag = 0;   //magnitude of right stick
        double yaw = 0;             //yaw from IMU
        double yaw0 = 0;            //zero direction for the bot

        double transX = 0;          //commands for where we want the bot to end up
        double transY = 0;
        double angleCommand = 0;
        
        double MAX_ANGLE_STEP = 15;  //this is the maximum amount that our robot will attempt to rotate in a single time step. (in radians)

        int loopNum = 0;
        int prevMod = 0;            // tbh this rlly shouldn't exist and is a very bad workaround for a crashing issue but idk how to fix it the right way so you'll just have to live with this


        public class SwerveModule{
            public int id;
            public double x;
            public double y;
            public double angle;
            public int driveMotorID;
            public int steerMotorID;
            public int encAddress;
            public double xOffset;
            public double yOffset;
            private double frameSize;
            public double drive;
            public double commandedAngle;

            public SwerveModule(int idNum, int driveMotorIDnum, int steerMotorIDnum, int encAddressNum, double frameSizeNum) {
                id = idNum;
                driveMotorID = driveMotorIDnum;
                steerMotorID = steerMotorIDnum;
                encAddress = encAddressNum;
                frameSize = frameSizeNum;
                // initialize frame offsets
                switch(driveMotorIDnum) {
                    case 0:
                        Debug.WriteLine("Warning, swerve module was initialized with ID 0. Valid IDs are 1-4.");
                        break;
                    case 1:
                        // i really should be using FRAME_WIDTH here 
                        xOffset = -1 * frameSize / 2;
                        yOffset = frameSize / 2;
                        break;
                    case 2:
                        xOffset = frameSize / 2;
                        yOffset = frameSize / 2;
                        break;
                    case 3:
                        xOffset = frameSize / 2;
                        yOffset = -1 * frameSize / 2;
                        break;
                    case 4:
                        xOffset = -1 * frameSize / 2;
                        yOffset = -1 * frameSize / 2;
                        break;
                }
                x = xOffset; y = yOffset; // just gonna init the x and y positions to the
            }
        }

        SwerveModule sm0 = new SwerveModule(0, 0, 0, 0, 0);     // module 0 shouldn't actually be used, it's just a default value for when something goes wrong
        SwerveModule sm1 = new SwerveModule(1, 1, 2, 21, 0.446);
        SwerveModule sm2 = new SwerveModule(2, 3, 4, 22, 0.446);
        SwerveModule sm3 = new SwerveModule(3, 5, 6, 23, 0.446);
        SwerveModule sm4 = new SwerveModule(4, 7, 8, 24, 0.446);

        private static System.Timers.Timer JoystickTimer;

        public Form1() {
            TimerInit();
            CheckForIllegalCrossThreadCalls = false;
            InitializeComponent();
            disableControls();
            getAvailableComPorts();


            foreach (string port in ports) {
                comboBox1.Items.Add(port);
                Console.WriteLine(port);
                if (ports[0] != null) {
                    comboBox1.SelectedItem = ports[0];
                }
            }

        }

        private void TimerInit() {
            JoystickTimer = new System.Timers.Timer(50);
            JoystickTimer.Elapsed += OnTimedEvent;
            JoystickTimer.AutoReset = true;
            JoystickTimer.Enabled = true;
        }

        private void OnTimedEvent(Object source, ElapsedEventArgs e) {
            loopNum++;

            // if the override checkboxes are checked then use those as the stick inputs
            if (checkBox1.Checked) {
                steerStickX = trackBar1.Value * 0.2;
                steerStickY = trackBar2.Value * 0.2;
            } else {
                steerStickX = 0;
                steerStickY = 0;
            }
            if (checkBox2.Checked) {
                driveStickX = trackBar4.Value * 0.2;
                driveStickY = trackBar3.Value * 0.2;
            } else {
                driveStickX= 0;
                driveStickY= 0;
            }

            // calculate stick magnitudes
            steerStickMag = MAGNITUDE(steerStickX, steerStickY);
            driveStickMag = MAGNITUDE(driveStickX, driveStickY);

            // if we're in the deadzone, set positions to 0 and don't updates angles. outside the deadzone we still update the stick angles.
            if (Math.Abs(steerStickMag) < LEFT_STICK_DEADZONE) {
                steerStickX = 0;
                steerStickY = 0;
            } else {
                steerStickAngle = NormalizeAngle(rad2deg(Math.Atan2(steerStickY, steerStickX)) + 90);
            }
            if (Math.Abs(driveStickMag) < RIGHT_STICK_DEADZONE) {
                driveStickX = 0;
                driveStickY = 0;
            } else {
                driveStickAngle = NormalizeAngle(rad2deg(Math.Atan2(driveStickY, driveStickX)) + 90);
            }

            // now we set the commanded angle. if the commanded angle is greater than the maximum allowed angle adjustment, clamp it to the max.
            if (Math.Abs(steerStickAngle - (yaw + yaw0)) < MAX_ANGLE_STEP) {
                angleCommand = steerStickAngle;
            } else if (steerStickAngle - (yaw + yaw0) > 0 ) {
                angleCommand = yaw + yaw0 + MAX_ANGLE_STEP;
            } else {
                angleCommand = yaw + yaw0 - MAX_ANGLE_STEP;
            }

            // now we set the translation commands
            // TODO: make this logarithmic instead of linear
            transX = driveStickX*ZOOM_ZOOM;
            transY = driveStickY*ZOOM_ZOOM;

            // we now know how much we want to rotate and translate. time to generate swerve commands
            for (int i = 1; i < 5; i++) {
                
                double[] dummyCommand = CalcPosAngleDrive(GetModule(i));
                //if(loopNum % 20 == 0) {
                //    Debug.WriteLine("Debug info for module " + i + ":");
                //    Debug.WriteLine("dummy command x: " + dummyCommand[0]);
                //    Debug.WriteLine("dummy command y: " + dummyCommand[1]);
                //    Debug.WriteLine("dummy command pointing angle: " + dummyCommand[2]);
                //    Debug.WriteLine("dummy command mag: " + dummyCommand[3]);
                //    Debug.WriteLine("======\n");
                //}
                

                GetModule(i).commandedAngle = dummyCommand[2];
                GetModule(i).drive = dummyCommand[3];
                SendMotorCommands(GetModule(i));
            }
            

            // code for updating swerve module debug UI
            if(comboBox2.SelectedItem != null) {
                Debug.WriteLine("selected item is: " + comboBox2.SelectedItem);
                try {
                    int currentModule = int.Parse(comboBox2.SelectedItem.ToString());
                    Bitmap frameVisual2 = new Bitmap(pictureBox4.Image);
                    pictureBox3.Image = RotateImg(frameVisual2, (float)GetModule(currentModule).commandedAngle);
                    frameVisual2.Dispose();
                    textBox5.Text = "Angle: " + GetModule(currentModule).angle.ToString();
                    textBox6.Text = "Drive: " + GetModule(currentModule).drive.ToString();
                    textBox7.Text = "AngCom: " + GetModule(currentModule).commandedAngle.ToString();
                } catch {
                    Debug.WriteLine("uh oh u just tried to access something u weren't supposed to. smh my head.");
                }
                
            }
            // code for updating overall robot UI
            textBox8.Text = "Angle: " + NormalizeAngle(yaw + yaw0).ToString();
            textBox9.Text = "Desired Angle: " + NormalizeAngle(angleCommand).ToString();
            textBox10.Text = "TransX: " + transX.ToString();
            textBox11.Text = "TransY: " + transY.ToString();
            textBox12.Text = "Steer Stick Angle: " + NormalizeAngle(steerStickAngle).ToString();
            textBox13.Text = "Yaw0: " + yaw0.ToString();

        }

        private void button1_Click(object sender, EventArgs e) {
            if (!isConnected) {
                connectToArduino();
            } else {
                disconnectFromArduino();
            }
        }

        void getAvailableComPorts() {
            ports = SerialPort.GetPortNames();
        }

        private void connectToArduino() {
            isConnected = true;
            string selectedPort = comboBox1.GetItemText(comboBox1.SelectedItem);
            port = new SerialPort(selectedPort, 115200, Parity.None, 8, StopBits.One);
            port.DataReceived += new SerialDataReceivedEventHandler(port_DataReceived);
            port.Open();
            port.Write("#STAR\n");
            button1.Text = "Disconnect";
            enableControls();
        }

        private void disconnectFromArduino() {
            isConnected = false;
            port.Write("#STOP\n");
            port.Close();
            button1.Text = "Connect";
            disableControls();
            resetDefaults();
        }

        private void button2_Click(object sender, EventArgs e) {
            if (isConnected) {
                port.Write("#" + textBox1.Text + "\n");
            }
        }

        private void enableControls() {
            textBox1.Enabled = true;
            groupBox1.Enabled = true;
            groupBox3.Enabled = true;
            groupBox4.Enabled = true;
            groupBox5.Enabled = true;
            groupBox6.Enabled = true;
            groupBox7.Enabled = true;
        }

        private void disableControls() {
            textBox1.Enabled = false;
            groupBox1.Enabled = false;
            groupBox3.Enabled = false;
            groupBox4.Enabled = false;
            groupBox5.Enabled = false;
            groupBox6.Enabled = false;
            groupBox7.Enabled = false;
        }

        private void resetDefaults() {
            textBox1.Text = "";
        }

        private void button3_Click(object sender, EventArgs e) {
            SendMessage("RST", "1");
            WriteToDebugBox("Resetting IMU...\n");
            Debug.Write("Restting IMU...\n");
        }

        public double MAGNITUDE(double x, double y) {
            return Math.Sqrt(x * x + y * y);
        }

        public static Bitmap RotateImg(Bitmap bmp, float angle) {

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

        private SwerveModule GetModule(int ID) {
            // returns the swerve module object with the corresponding ID. defaults to sm0, which shouldn't ever actually happen.
            switch (ID) {
                case 1:
                    return sm1;
                case 2:
                    return sm2;
                case 3:
                    return sm3;
                case 4:
                    return sm4;
            }
            return sm0;
        }

        private void port_DataReceived(object sender, SerialDataReceivedEventArgs e) {
            // Show all the incoming data in the port's buffer

            string tempStr = port.ReadExisting();
            string message = "";
            if (tempStr[tempStr.Length - 1] == '\n') {
                message = receivedData + tempStr;
                textBox1.Text = message;
                receivedData = "";
                switch (GetID(message)) {
                    case "YAW":
                        Bitmap frameVisual = new Bitmap(pictureBox2.Image);
                        try {
                            yaw = NormalizeAngle(double.Parse(GetPayload(message)));
                            pictureBox1.Image = RotateImg(frameVisual, (float)yaw);
                        } catch {
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
            } else {
                receivedData += tempStr;
            }

        }

        public String GetID(String str) {
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
             * M1A - module 1 angle (float, radians)
             * M1D - module 1 drive (float)
             * M2A - module 2 angle (float, radians)
             * M2D - module 2 drive (float)
             * M3A - module 3 angle (float, radians)
             * M3D - module 3 drive (float)
             * M4A - module 4 angle (float, radians)
             * M4D - module 4 drive (float)
             * 
             */
            if (str[0] != '#' || str[str.Length - 1] != '\n' || str.Length > 256) {
                //if the message does not start and end correctly then the message is invalid and we toss it.
                return "INV";
            } else {
                return str.Substring(1, 3);
            }
        }

        public bool SendMessage(String ID, String payload) {
            if(port != null && port.IsOpen) {
                port.Write("#" + ID + payload + "\n");
                return true;
            } else {
                return false;
            }

        }

        public String GetPayload(String str) {
            if (str[0] != '#' || str[str.Length - 1] != '\n' || str.Length > 256) {
                //if the message does not start and end correctly then the message is invalid and we toss it.
                //it is technically possible that we might want to send a payload that happens to exactly match
                //the binary representation of "INVALID_MESSAGE" but like, too bad. you dont get to do that now.
                return "INVALID_MESSAGE";
            } else {

                return str.Substring(4, str.Length - 4);
            }
        }

        private void WriteToDebugBox(String msg) {
            Debug.Print(msg);
            if (textBox2.Text.Length > 300) {
                textBox2.Text = "";
            }
            textBox2.Text += "\n" + msg;
        }

        private void textBox3_TextChanged(object sender, EventArgs e) {
            port.Write(textBox3.Text);
        }

        private double deg2rad(double angle) {
            // can't believe this isn't just already in Math.h smh my head
            return angle * Math.PI / 180;
        }

        private double rad2deg(double angle) {
            // can't believe this isn't just already in Math.h smh my head
            return angle * 180 / Math.PI;
        }

        private Vector<double> Rotate2(Vector<double> v, double theta) {
            // takes in a 2x1 vector and angle (in degrees), returns that vector rotated by the angle
            double angle = deg2rad(theta);
            Matrix<double> rotMat = DenseMatrix.OfArray(new double[,] {
                {Math.Cos(angle), -Math.Sin(angle)},
                {Math.Sin(angle), Math.Cos(angle)}
            });
            return rotMat.Multiply(v);
        }

        private double[] CalcPosAngleDrive(SwerveModule mod) {
            // Calculates desired XY position of swerve module, pointing angle between current and desired position, and drive magnitude.
            // return format: { x, y, pointing angle, magnitude }
            
            // calculate the xy position of the module relative to the center of the robot
            Vector<double> oldPos = Rotate2(DenseVector.OfArray(new double[] { mod.xOffset, mod.yOffset }), NormalizeAngle(yaw + yaw0));

            // calculate where we want the module to end up
            Vector<double> newPos = Rotate2(DenseVector.OfArray(new double[] { mod.xOffset, mod.yOffset }), NormalizeAngle(angleCommand));
            newPos[0] += transX;
            newPos[1] += transY;

            Vector<double> pointingVector = -newPos + oldPos;
            double[] returnVar = { newPos[0], newPos[1], rad2deg(Math.Atan2(pointingVector[1], pointingVector[0])), MAGNITUDE(pointingVector[0], pointingVector[1]) };
            return returnVar;
        }

        private void SendMotorCommands(SwerveModule mod) {
            SendMessage("M" + mod.id.ToString() + "A", mod.commandedAngle.ToString());
            SendMessage("M" + mod.id.ToString() + "D", mod.drive.ToString());
        }

        private double NormalizeAngle(double angle) {
            // remaps the input angle to ensure it's always in the range { -180, 180 }.
            // i.e. 270 gets remapped to -90
            // (input and return angle are in degrees)

            // first we get rid of any complete revolutions with a simple modulo
            double num = angle % 360;
            
            // then we do the remap
            if (num > 180) {
                num = -(180 + (180 + num));
            } else if(num < -180) {
                num = (180 - (-180 - num));
            }
            return num;
        }
    }
}
