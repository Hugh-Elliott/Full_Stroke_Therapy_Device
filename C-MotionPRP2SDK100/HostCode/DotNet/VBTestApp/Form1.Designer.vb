<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class Form1
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        If disposing AndAlso components IsNot Nothing Then
            components.Dispose()
        End If
        MyBase.Dispose(disposing)
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Me.ButtonCOM = New System.Windows.Forms.Button()
        Me.LabelVersionChip = New System.Windows.Forms.Label()
        Me.ButtonTCP = New System.Windows.Forms.Button()
        Me.LabelDefaultsCOM = New System.Windows.Forms.Label()
        Me.TCPMessage = New System.Windows.Forms.TextBox()
        Me.TCPReply = New System.Windows.Forms.Label()
        Me.ListBox1 = New System.Windows.Forms.ListBox()
        Me.ButtonTCPSend = New System.Windows.Forms.Button()
        Me.CAN = New System.Windows.Forms.Button()
        Me.GroupBox1 = New System.Windows.Forms.GroupBox()
        Me.GroupBox2 = New System.Windows.Forms.GroupBox()
        Me.GroupBox3 = New System.Windows.Forms.GroupBox()
        Me.LabelVersionCME = New System.Windows.Forms.Label()
        Me.GroupBox3.SuspendLayout()
        Me.SuspendLayout()
        '
        'ButtonCOM
        '
        Me.ButtonCOM.Location = New System.Drawing.Point(74, 54)
        Me.ButtonCOM.Name = "ButtonCOM"
        Me.ButtonCOM.Size = New System.Drawing.Size(75, 23)
        Me.ButtonCOM.TabIndex = 1
        Me.ButtonCOM.Text = "COM"
        Me.ButtonCOM.UseVisualStyleBackColor = True
        '
        'LabelVersionChip
        '
        Me.LabelVersionChip.AutoSize = True
        Me.LabelVersionChip.Location = New System.Drawing.Point(207, 50)
        Me.LabelVersionChip.Name = "LabelVersionChip"
        Me.LabelVersionChip.Size = New System.Drawing.Size(55, 13)
        Me.LabelVersionChip.TabIndex = 1
        Me.LabelVersionChip.Text = "IC Version"
        '
        'ButtonTCP
        '
        Me.ButtonTCP.Location = New System.Drawing.Point(74, 130)
        Me.ButtonTCP.Name = "ButtonTCP"
        Me.ButtonTCP.Size = New System.Drawing.Size(75, 23)
        Me.ButtonTCP.TabIndex = 3
        Me.ButtonTCP.Text = "Ethernet"
        Me.ButtonTCP.UseVisualStyleBackColor = True
        '
        'LabelDefaultsCOM
        '
        Me.LabelDefaultsCOM.AutoSize = True
        Me.LabelDefaultsCOM.Location = New System.Drawing.Point(207, 103)
        Me.LabelDefaultsCOM.Name = "LabelDefaultsCOM"
        Me.LabelDefaultsCOM.Size = New System.Drawing.Size(46, 13)
        Me.LabelDefaultsCOM.TabIndex = 5
        Me.LabelDefaultsCOM.Text = "Defaults"
        '
        'TCPMessage
        '
        Me.TCPMessage.Location = New System.Drawing.Point(210, 361)
        Me.TCPMessage.Name = "TCPMessage"
        Me.TCPMessage.Size = New System.Drawing.Size(104, 20)
        Me.TCPMessage.TabIndex = 8
        '
        'TCPReply
        '
        Me.TCPReply.AutoSize = True
        Me.TCPReply.Location = New System.Drawing.Point(207, 345)
        Me.TCPReply.Name = "TCPReply"
        Me.TCPReply.Size = New System.Drawing.Size(55, 13)
        Me.TCPReply.TabIndex = 9
        Me.TCPReply.Text = "TCPReply"
        '
        'ListBox1
        '
        Me.ListBox1.FormattingEnabled = True
        Me.ListBox1.Location = New System.Drawing.Point(18, 106)
        Me.ListBox1.Name = "ListBox1"
        Me.ListBox1.Size = New System.Drawing.Size(120, 160)
        Me.ListBox1.TabIndex = 4
        '
        'ButtonTCPSend
        '
        Me.ButtonTCPSend.Location = New System.Drawing.Point(74, 353)
        Me.ButtonTCPSend.Name = "ButtonTCPSend"
        Me.ButtonTCPSend.Size = New System.Drawing.Size(75, 23)
        Me.ButtonTCPSend.TabIndex = 5
        Me.ButtonTCPSend.Text = "TCP Send"
        Me.ButtonTCPSend.UseVisualStyleBackColor = True
        '
        'CAN
        '
        Me.CAN.Location = New System.Drawing.Point(74, 92)
        Me.CAN.Name = "CAN"
        Me.CAN.Size = New System.Drawing.Size(75, 23)
        Me.CAN.TabIndex = 2
        Me.CAN.Text = "CAN"
        Me.CAN.UseVisualStyleBackColor = True
        '
        'GroupBox1
        '
        Me.GroupBox1.Location = New System.Drawing.Point(51, 24)
        Me.GroupBox1.Name = "GroupBox1"
        Me.GroupBox1.Size = New System.Drawing.Size(122, 155)
        Me.GroupBox1.TabIndex = 0
        Me.GroupBox1.TabStop = False
        Me.GroupBox1.Text = "Interface"
        '
        'GroupBox2
        '
        Me.GroupBox2.Location = New System.Drawing.Point(44, 319)
        Me.GroupBox2.Name = "GroupBox2"
        Me.GroupBox2.Size = New System.Drawing.Size(344, 80)
        Me.GroupBox2.TabIndex = 4
        Me.GroupBox2.TabStop = False
        Me.GroupBox2.Text = "Generic send and receive to a CME user application"
        '
        'GroupBox3
        '
        Me.GroupBox3.Controls.Add(Me.LabelVersionCME)
        Me.GroupBox3.Controls.Add(Me.ListBox1)
        Me.GroupBox3.Location = New System.Drawing.Point(192, 24)
        Me.GroupBox3.Name = "GroupBox3"
        Me.GroupBox3.Size = New System.Drawing.Size(196, 283)
        Me.GroupBox3.TabIndex = 13
        Me.GroupBox3.TabStop = False
        Me.GroupBox3.Text = "Output"
        '
        'LabelVersionCME
        '
        Me.LabelVersionCME.AutoSize = True
        Me.LabelVersionCME.Location = New System.Drawing.Point(15, 53)
        Me.LabelVersionCME.Name = "LabelVersionCME"
        Me.LabelVersionCME.Size = New System.Drawing.Size(68, 13)
        Me.LabelVersionCME.TabIndex = 14
        Me.LabelVersionCME.Text = "CME Version"
        '
        'Form1
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 13.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(440, 413)
        Me.Controls.Add(Me.CAN)
        Me.Controls.Add(Me.ButtonTCPSend)
        Me.Controls.Add(Me.TCPReply)
        Me.Controls.Add(Me.TCPMessage)
        Me.Controls.Add(Me.LabelDefaultsCOM)
        Me.Controls.Add(Me.ButtonTCP)
        Me.Controls.Add(Me.LabelVersionChip)
        Me.Controls.Add(Me.ButtonCOM)
        Me.Controls.Add(Me.GroupBox1)
        Me.Controls.Add(Me.GroupBox2)
        Me.Controls.Add(Me.GroupBox3)
        Me.Name = "Form1"
        Me.Text = "Form1"
        Me.GroupBox3.ResumeLayout(False)
        Me.GroupBox3.PerformLayout()
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub
    Friend WithEvents ButtonCOM As System.Windows.Forms.Button
    Friend WithEvents LabelVersionChip As System.Windows.Forms.Label
    Friend WithEvents ButtonTCP As System.Windows.Forms.Button
    Friend WithEvents LabelDefaultsCOM As System.Windows.Forms.Label
    Friend WithEvents TCPMessage As System.Windows.Forms.TextBox
    Friend WithEvents TCPReply As System.Windows.Forms.Label
    Friend WithEvents ListBox1 As System.Windows.Forms.ListBox
    Friend WithEvents ButtonTCPSend As System.Windows.Forms.Button
    Friend WithEvents CAN As System.Windows.Forms.Button
    Friend WithEvents GroupBox1 As GroupBox
    Friend WithEvents GroupBox2 As GroupBox
    Friend WithEvents GroupBox3 As GroupBox
    Friend WithEvents LabelVersionCME As Label
End Class
