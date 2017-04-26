using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using System.Runtime.Serialization.Formatters.Binary;
using System.IO;
using System.IO.Ports;

namespace tune
{
    //public class CGrafPanel : Panel
    //{
    //    public CGrafPanel()
    //    {
    //        this.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
    //        | System.Windows.Forms.AnchorStyles.Left)
    //        | System.Windows.Forms.AnchorStyles.Right)));
    //        this.Location = new System.Drawing.Point(1, 1);
    //        //this.AutoSize = false;
    //        this.BackColor = System.Drawing.Color.LightGray;
    //    }
    //}

    [Serializable]
    public class CarTurboData
    {
        [Serializable]
        public class CarTable
        {
            public string name = "cartab";
            public const int PointCount = 51;//51
            public const int MaxPoints = 250;//250
            public const float MaxValue = 5.0F;
            public const int PointStep = MaxPoints / (PointCount - 1);
            public Point[] points = new Point[PointCount];
        } //class CarTable
        public List<CarTable> tables = new List<CarTable>();
    } //class CarTurboData

    public class CGrafTabPage : TabPage
    {
      private BufferedGraphicsContext context;
      private BufferedGraphics grafx;

      private Point[] points_def = new Point[CarTurboData.CarTable.PointCount]; // default
      private Point[] pointsGreedm = new Point[CarTurboData.CarTable.PointCount]; // greed in zoom
      private Point[] points = new Point[CarTurboData.CarTable.PointCount];  // real points for controller
      private Point[] pointsm = new Point[CarTurboData.CarTable.PointCount]; // points in zoom
      private RectangleF[] move_pointsm = new RectangleF[CarTurboData.CarTable.PointCount]; // regions for capture
      private PointF Km;
      private const int SizeMovePoint = 7;
      //
      private bool isinside = false; // чи попали мишкою в точку
      private int irec; // номер точки в масиві
      private bool isDragging = false;
      private int deltaY;
      private int Ydown;
      private Point[] points_fix;
        //

        public Point[] GetPoints()
        {
            return points;
        }

        public void SetPoints(Point[] _points)
        {
            points = _points;
            EventArgs e = new EventArgs();
            this.BeginInvoke(new MethodInvoker(() => gtp_Resize(this, e) ));
            //this.gtp_Resize(this, e);
        }

        private struct ТМargin
        {
            public int left, right, top, bottom;
            public ТМargin(int Left, int Right, int Top, int Bottom)
            {
                left = Left;  right = Right;  top = Top; bottom = Bottom;
            }
        }; 
        private ТМargin TabMargin = new ТМargin(40,30,30,40);
        private int StepMark = 5; //point

        public CGrafTabPage(int index, string tablename)
        {
            GeneratePointsDefault();
            this.Location = new System.Drawing.Point(4, 22);
            this.Name = "tabPage" + index.ToString();
            this.Padding = new System.Windows.Forms.Padding(3);
            //this.Size = new System.Drawing.Size(740, 347);
            this.TabIndex = index;
            this.Text = tablename;
            this.UseVisualStyleBackColor = true;
            this.MouseMove  += new System.Windows.Forms.MouseEventHandler(this.gtp_MouseMove);
            this.MouseDown  += new System.Windows.Forms.MouseEventHandler(this.gtp_MouseDown);
            this.MouseUp    += new System.Windows.Forms.MouseEventHandler(this.gtp_MouseUp);
            this.Resize     += new System.EventHandler(this.gtp_Resize);
            context = new BufferedGraphicsContext();
            context.MaximumBuffer = new Size(this.Width + 1, this.Height + 1);
            grafx = context.Allocate(this.CreateGraphics(),
                 new Rectangle(0, 0, this.Width, this.Height));
            DrawToBuffer(grafx.Graphics);
        }
                   
        ~CGrafTabPage()
        {
            //context.Dispose();
        }

        private void GeneratePointsDefault()
        {
            for (int i = 0; i < CarTurboData.CarTable.PointCount; i++)
            {
                int x = i * CarTurboData.CarTable.PointStep;
                points_def[i] = new Point(x,x);
                points[i] = new Point(x, x);
            }
        }

        private void GeneratePointsKm()
        {
            Km = GetKm();
            for (int i = 0; i < points_def.Length; i++)
            {
                pointsGreedm[i] = new Point((int)(points_def[i].X * Km.X) + TabMargin.left,
                    (int)(points_def[i].Y * Km.Y) + TabMargin.top);
                pointsm[i] = new Point((int)(points[i].X * Km.X) + TabMargin.left,
                    (int)((points_def[points_def.Length - 1].Y - points[i].Y) * Km.Y) + TabMargin.top);
                move_pointsm[i] = new RectangleF(pointsm[i].X - SizeMovePoint / 2, pointsm[i].Y - SizeMovePoint / 2, SizeMovePoint, SizeMovePoint);
            }
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            //base.OnPaint(e);
            //DrawToBuffer(grafx.Graphics);
            grafx.Render(e.Graphics);
            
            //
            //Graphics g = e.Graphics;
            //DrawGrid(g);
            //Rectangle rec = this.ClientRectangle;
            //g.DrawString(String.Format("Привіт GDI+ : {0}", rec.ToString()), new Font("Times New Roman", 20), Brushes.Green, 50, 40);
            //DrawData(g);
            //
        }

        private void DrawToBuffer(Graphics g)
        {
            g.FillRectangle(Brushes.White, 0, 0, this.Width, this.Height);
            DrawGrid(g);
            Rectangle rec = this.ClientRectangle;
            g.DrawString("Ваш автомобіль буде літати", new Font("Times New Roman", 20), Brushes.Blue, 80, 5);
            DrawData(g);
        }

        private PointF GetKm()
        { 
            PointF pt = new PointF(
               ((this.ClientRectangle.Width - (TabMargin.left + TabMargin.right) ) / (float)CarTurboData.CarTable.MaxPoints),
              ((this.ClientRectangle.Height - (TabMargin.top + TabMargin.bottom)) / (float)CarTurboData.CarTable.MaxPoints)
            );
            if (pt.X < 1.0) pt.X = 1.0F;
            if (pt.Y < 1.0) pt.Y = 1.0F;
            return pt;
        }

        private void DrawGrid(Graphics g)
        {
            for (int i = 0; i < pointsGreedm.Length; i++)
            {
                g.DrawLine(Pens.Gray, pointsGreedm[i].X, pointsGreedm[0].Y, pointsGreedm[i].X, pointsGreedm[pointsGreedm.Length - 1].Y);
                g.DrawLine(Pens.Gray, pointsGreedm[0].X, pointsGreedm[i].Y, pointsGreedm[pointsGreedm.Length - 1].X, pointsGreedm[i].Y);
            }
            //draw middle line
            Pen ml_pen = new Pen(Color.Green, 1);
            for (int i = 0; i < pointsGreedm.Length - 1 ; i++)
            {
                g.DrawLine(ml_pen, pointsGreedm[i].X, TabMargin.top + pointsGreedm[pointsGreedm.Length - 1].Y - pointsGreedm[i].Y,
                    pointsGreedm[i + 1].X, TabMargin.top + pointsGreedm[pointsGreedm.Length - 1].Y - pointsGreedm[i + 1].Y);
            }
            // draw axes
            Pen pen = new Pen(Color.Green, 3);
            g.DrawLine(pen, pointsGreedm[0].X - 10, pointsGreedm[pointsGreedm.Length - 1].Y,
                pointsGreedm[pointsGreedm.Length - 1].X + 10, pointsGreedm[pointsGreedm.Length - 1].Y);
            g.DrawLine(pen, pointsGreedm[0].X , pointsGreedm[0].Y - 10,
                pointsGreedm[0].X, pointsGreedm[pointsGreedm.Length - 1].Y + 10);
            // draw axes mark and legends
            int lmark = 3;
            int rlmark;
            Font font = new Font("Times New Roman", 10);
            Brush brash = Brushes.Green;
            int Ylegend = pointsGreedm[pointsGreedm.Length - 1].Y + 5;
            int Xlegend = pointsGreedm[0].X - 32;
            for (int i = StepMark; i < points_def.Length; i += StepMark)
            {
                if ( (i & 1) == 1 )
                {
                    rlmark = lmark;
                }
                else
                {
                    rlmark = lmark * 2;
                }

                g.DrawString(String.Format("{0:F1}V", ((i * CarTurboData.CarTable.MaxValue) / (CarTurboData.CarTable.PointCount - 1))), font, brash, pointsGreedm[i].X - 10, Ylegend); //x
                g.DrawString(String.Format("{0:F1}V", ((i * CarTurboData.CarTable.MaxValue) / (CarTurboData.CarTable.PointCount - 1))), font, brash, Xlegend, pointsGreedm[CarTurboData.CarTable.PointCount-1-i].Y - 7); //y

                g.DrawLine(pen, pointsGreedm[i].X, pointsGreedm[pointsGreedm.Length - 1].Y - rlmark,
                                pointsGreedm[i].X, pointsGreedm[pointsGreedm.Length - 1].Y + rlmark);

                g.DrawLine(pen, pointsGreedm[0].X - rlmark, pointsGreedm[i].Y,
                                pointsGreedm[0].X + rlmark, pointsGreedm[i].Y);

            }
        }

        private void DrawData(Graphics g)
        {
           g.DrawLines(new Pen(Color.Red, 2), pointsm);
           Pen circlePen = new Pen(Color.Red, 3);
           for (int i = 0; i < move_pointsm.Length; i++)
           {
               g.DrawEllipse(circlePen, move_pointsm[i]);
           }
        }

        private void gtp_MouseDown(object sender, MouseEventArgs e)
        {
            if (isinside)
            {
                isDragging = true;
                //deltaY = e.Y - (int)move_pointsm[irec].Y;
                Ydown = e.Y;
                points_fix = (Point[])points.Clone();
            }
        }

        private bool ControlActivePoint(Point point)
        {
            isinside = false;
            for (int i = 0; i < move_pointsm.Length; i++)
            {
                if (move_pointsm[i].Contains(point))
                {
                    isinside = true;
                    irec = i;
                    break;
                }
            }
            return isinside;
        }

        private void gtp_MouseMove(object sender, MouseEventArgs e)
        {
            Point mousePt = new Point(e.X, e.Y);
            if (ControlActivePoint(mousePt) || isDragging) this.Cursor = Cursors.Hand;
            else this.Cursor = Cursors.Arrow;

            if (isDragging)
            {
              deltaY = (int)((Ydown - e.Y) / Km.Y);
              //points[irec].Y = points_fix[irec].Y + deltaY;
              AddingChange(irec, deltaY);
              GeneratePointsKm();
              //move_pointsm[irec].Y = e.Y - deltaY;
              //pointsm[irec].Y = (int)move_pointsm[irec].Y + SizeMovePoint / 2;
              //points[irec].Y = points_def[points_def.Length - 1].Y - (int)((pointsm[irec].Y - TabMargin.top) / Km.Y);
                
              //Invalidate();
              //DrawToBuffer(grafx.Graphics);
              //this.Refresh();
              //Graphics g = Graphics.FromHdc(this.Handle);
              //DrawData(g);
              //g.Dispose();
            }
        }

        public void AddingChange(int n, int val) 
        {
            /*const float k = 1.9f;
            float del;
            for (int i = 0; i < points.Length; i++)
            {
              del = (float)Math.Pow((n - i) * k, 2);
              if (del < 1.0F) del = 1.0F;
              points[i].Y = points_fix[i].Y + (int)(val / del);
              if (points[n].Y > 255) points[n].Y = 255;
              else if (points[n].Y < 0) points[n].Y = 0;
            }*/
            points[n].Y = points_fix[n].Y + val;
            if (points[n].Y > 255) points[n].Y = 255;
            else if (points[n].Y < 0) points[n].Y = 0;
        }

        private void gtp_MouseUp(object sender, MouseEventArgs e)
        {
            isDragging = false;
            DrawToBuffer(grafx.Graphics);
            this.Refresh();
        }

        private void gtp_Resize(object sender, EventArgs e)
        {
            if (this.Height < 1) return; // без цього виникае виключення при виклику grafx = context.Allocate(this.CreateGraphics(), .. 
            GeneratePointsKm();
            // Re-create the graphics buffer for a new window size.
            context.MaximumBuffer = new Size(this.Width + 1, this.Height + 1);
            if (grafx != null)
            {
                grafx.Dispose();
                grafx = null;
            }
            grafx = context.Allocate(this.CreateGraphics(),
                new Rectangle(0, 0, this.Width, this.Height));

            // Cause the background to be cleared and redraw.
            DrawToBuffer(grafx.Graphics);
            this.Refresh();
        }

    } //class CGrafTabPage

    public class CTurboControl : TabControl
    {
        const string s_noname = "Без імені";
        private string filename = s_noname;
        public string FileName { get {return filename;} }
        public List<CGrafTabPage> tables = new List<CGrafTabPage>();
        public static string[] tabs = new string[] { "Паливо - Тиск", "Повітря - MAF", "Повітря - Тиск", "резерв" };
        
        public CTurboControl()
        { 
            int i = 0;
            foreach (string s in tabs)
            {
                CGrafTabPage gtp = new CGrafTabPage(i++, s);
                tables.Add(gtp);
                Controls.Add(gtp);
            }
            //
            this.Dock = System.Windows.Forms.DockStyle.Fill;
            this.Location = new System.Drawing.Point(0, 0);
            this.Name = "tabControlGraf";
            this.SelectedIndex = 0;
            //this.Size = new System.Drawing.Size(600, 373);
            this.TabIndex = 0;
            //
            //this.BackColor = System.Drawing.SystemColors.Control;
            //this.ForeColor = System.Drawing.SystemColors.ControlText;

        }

        public void ClearData() // create new tabs
        {
            this.RemoveAll();
            tables.Clear();
            //
            this.filename = s_noname;
            int i = 0;
            foreach (string s in tabs)
            {
                CGrafTabPage gtp = new CGrafTabPage(i++, s);
                tables.Add(gtp);
                Controls.Add(gtp);
            }
        }

        public byte [] GetTable (string TableName) {
            foreach (CGrafTabPage gtp in tables)
            {
                if (TableName == gtp.Text)
                {
                    Point[] points = gtp.GetPoints();
                    byte[] table = new byte[points.Length];
                    for (int i = 0; i < points.Length; i++)
                    {
                        table[i] = (byte)points[i].Y;
                    }
                    return table;
                }
            }
            string ex =  string.Format("Таблиці {0} не існує", TableName);
            throw new Exception(ex);
        }

        public void SetTable(string TableName, byte[] data)
        {
            foreach (CGrafTabPage gtp in tables)
            {
                if (TableName == gtp.Text)
                {
                    Point[] points = gtp.GetPoints();
                    for (int i = 0; i < points.Length; i++)
                    {
                        points[i].Y = (int)data[i];
                    }
                    gtp.SetPoints(points);
                    return;
                }
            }
            string ex = string.Format("Таблиці {0} не існує", TableName);
            throw new Exception(ex);
        }

        public CarTurboData GetData()
        {
            CarTurboData ctd = new CarTurboData();
            foreach (CGrafTabPage gtp in tables)
            {
                CarTurboData.CarTable ct = new CarTurboData.CarTable();
                ct.name = gtp.Text;
                ct.points = gtp.GetPoints();

                ctd.tables.Add(ct);
            }
            return ctd;
        }

        public void SetData(CarTurboData ctd)
        {
            this.RemoveAll();
            tables.Clear();
            //
            int i = 0;
            foreach (CarTurboData.CarTable ct in ctd.tables)
            {
                CGrafTabPage gtp = new CGrafTabPage(i++, ct.name);
                gtp.SetPoints(ct.points);
                tables.Add(gtp);
                Controls.Add(gtp);
            }

        }
        
        public void LoadFromFile(string FileName)
        {
            try
            {
                Stream fStream = File.OpenRead(FileName);
                BinaryFormatter binFormat = new BinaryFormatter();
                CarTurboData ctd = (CarTurboData)binFormat.Deserialize(fStream);
                fStream.Close();
                SetData(ctd);
                filename = FileName;
            }
            catch (IOException e)
            {
                MessageBox.Show(String.Format("Помилка вводу-виводу: {0}",e.Message), 
                    "Тюнінг авто", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            catch(System.Runtime.Serialization.SerializationException e) 
            { 
                MessageBox.Show(String.Format("Невірний формат файлу: {0}",e.Message), 
                    "Тюнінг авто", MessageBoxButtons.OK, MessageBoxIcon.Error);

            }
        }
        
        public void SaveToFile(string FileName)
        {
            CarTurboData ctd = this.GetData();
            try
            {
                Stream fStream = new FileStream(FileName, FileMode.Create, FileAccess.Write, FileShare.None);
                BinaryFormatter binFormat = new BinaryFormatter();
                binFormat.Serialize(fStream, ctd);
                fStream.Close();
                filename = FileName;
            }
            catch (IOException e)
            {
                MessageBox.Show(String.Format("Помилка вводу-виводу: {0}", e.Message), 
                    "Тюнінг авто", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
        

    } //class CTurboControl

} //namespace tune