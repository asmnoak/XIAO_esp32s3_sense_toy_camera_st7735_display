<p><H3>XIAO ESP32S3 SENSE ���g�p�����g�C�J����</H3></p>
<p>
XIAO ESP32S3 SENSE ���g�p�����g�C�J�����𐻍삵���̂ŏЉ��B<br>
XIAO ESP32S3 SENSE�ɂ́ASD�J�[�h���T�|�[�g����J�������W���[�����t�����Ă���B����XIAO ESP32S3 SENSE�ƃ��j�^�[�Ƃ���<br>
128x160/1.8�C���`��TFT�f�B�X�v���C�i�h���C�o�[��ST7735�j�j��g�ݍ��킹�ăg�C�J�����𐻍삵���B<br>
XIAO ESP32S3 SENS��WiFi�@�\�𗘗p���āA����̓u���E�U����s���B<br>
�J����Arduino IDE 2.1�ōs�����B<br>
XIAO ESP32S3 SENSE�ɂ��ẮA<a href="https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/">������</a>���Q�Ƃ̂��ƁB
</p>

<p><strong>�T�v</strong><br>
<p>�@�\�͈ȉ��̒ʂ�B</p>
<ul>
<li>JPG�̎ʐ^�iXGA�j���B��āAXIAO ESP32S3 SENSE������SD�J�[�h�ɕۑ��ł���B</li>
<li>JPG�̃X�g���[���摜�iXGA�j���u���E�U���猩����B</li>
<li>�ۑ����ꂽJPG�̎ʐ^���u���E�U���猩����B</li>
<li>���j�^�[��ʁiTFT�AQQVGA�j���T�|�[�g�B</li>
<li>�u���E�U���瑀�삵�āAJPG�̎ʐ^���B���B</li>
</ul>
</p>
<p><strong>H/W�\��</strong><br>
 �ESeeed Studio XIAO ESP32S3 SENSE - �R���g���[��<br>
 �ESPI�ڑ�&nbsp; TFT�f�B�X�v���C(ST7735)<br>
 �E�V���b�^�[�p�̃{�^���X�C�b�`�A�z���p�W�����v���C���[<br>
</p>
<p>
<img src="./inet_camera.JPG" width="440" height="440"><br>
���j�^�[������B�e�B�d���Ƃ��āA�O���̉����ɁA�P�O3�{�imax4.8V�j����̓d�r�{�b�N�X�����Ă���B�㕔�̃{�^���͎B�e�p�V���b�^�[�ł���B
</p>
<p><strong>�ڑ�</strong><br>
�e�R���|�[�l���g�̐ڑ��͈ȉ��̒ʂ�B<br>
</p>
<p>
<table> 
<tr>
<td>SPI</td><td>XIAO</td>
</tr>
<tr>
<td>TFT_MOSI</td><td>GPIO9</td>
</tr>
<tr>
<td>TFT_SCLK</td><td>GPIO7</td>
</tr>
<tr>
<td>TFT_CS</td><td>GPIO3</td>
</tr>
<tr>
<td>TFT_DC</td><td>GPIO4</td>
</tr>
<tr>
<td>TFT_RST</td><td>GPIO2</td>
</tr>
</table>
<p>�J�����̃V���b�^�[�i�{�^���X�C�b�`�j�́AGPIO1�ɐڑ�����B�{�^���X�C�b�`�̐ړ_�̂����A���������GND�ɐڑ�����B</p>
<p>XIAO ESP32S3 SENSE������SD�J�[�h���j�b�g�́AGPIO21�Ő��䂷��悤�ɂȂ��Ă���B<br>
�Ȃ��AGPIO21�́AXIAO ESP32S3 SENSE�̃r���g�C��LED�Ƌ��ʂł���B
</P>
</p>
<p><strong>�C���X�g�[��</strong><br>
<ol>
<li>�R�[�h���AZIP�`���Ń_�E�����[�h</li>
<li>�ǉ��̃��C�u�������AZIP�`���Ń_�E�����[�h�A���C�u�����}�l�[�W������C���X�g�[������</li>
 <ul>
  <li>Adafruit ST7735�i���C�u�����}�l�[�W�����猟���j</li>
  <li>TimeLib&nbsp;:&nbsp; https://github.com/PaulStoffregen/Time</li>
 </ul>
<li>ArduinoIDE����xiao_sense_vstream_st7735_display_jpgfile_qqvga_sd_master.ino���J��</li>
<li>�u���؁E�R���p�C���v�ɐ���������A��U�A�u���O��t���ĕۑ��v���s��</li>
<li>SSID��PASSWORD�̓A�N�Z�X�|�C���g�ɉ����āA�v���O������ύX���邱�ƁBWiFi�ɐڑ��ł��Ȃ��ꍇ�͓���s�ł���B</li>
</ol>
</p>
<p><strong>�u���E�U����̗��p</strong><br>
</p>
<p>�ڑ����ꂽIP�A�h���X�́AArduino�@IDE�̃V���A���o�͂ɕ\�������B<br>
����IP�A�h���X�Ƀu���E�U����ڑ��i�Ⴆ�΁hhttp://192.168.0.xx�h�Ǝw��j����ƁA���̉�ʂ��\�������B<br>
�ȈՂȃ��O�C���@�\���T�|�[�g���Ă��邽�߁A�p�X�R�[�h�̓��͂��K�v�ł���B�����l�́h123�h�ł���i���p���͕��G�Ȃ��̂ɏ��������邱�Ƃ����߂�j�B</p>
<p>
<img src="./inet_camera_1.png" width="660" height="260"><br>
</p>
<p>���O�C������ƁA���̉�ʂ��\�������B�htake a photo�h�{�^���������ƁA�ʐ^���B�e����AJPG�t�@�C����SD�J�[�h�ɕۑ������B<br>
�t�@�C�����́A�B�e�����ƃV�[�P���X�ԍ��ō\�������B��ʏ�ɂ́A�V�����t�@�C����20���܂Ń��X�g�����i�� �폜��SD�J�[�h���p�\�R���ɑ}���čs���j�B<br>
�t�@�C�������N���b�N����Ǝʐ^���u���E�U�ɕ\�������B
</p>
<p>"Stream Video�h���N���b�N����ƁA�u���E�U�ɃX�g���[���摜���\�������B�X�g���[���̕\�����I������ꍇ�́A�u���E�U���I������B</p>
<p>
<img src="./inet_camera_2.png" width="660" height="860"><br>
</p>
<p><strong>�኱�̉��</strong><br>
�E�R�[�h����WiFi��SSID��PASSWORD�́A���p���Ă��閳�����[�^�[�̂��̂��w�肷��B�v���O�����̕ύX�ŁA�������[�^�[��2��܂Ŏw�肷�邱�Ƃ��ł���B<br>
</p>
<p><strong>���ӎ���<br>
�E���p�̍ۂ́A���ȐӔC�ł��y���݂��������B</strong><br>
</p>
