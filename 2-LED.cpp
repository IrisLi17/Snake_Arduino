///fengh16. 17.06.29
///Arduino nano
///�ӿڣ�ҡ�ˣ�x-A1��y-A0��z-D2
///�ӿڣ�LED����DIN = 12, CS = 11, CLK = 10

#include "LedControl.h"

int PushX, PushY, PushZ, nowx, nowy, BodyX[128], BodyY[128], BodyLen, x, y, i, FoodX, FoodY, Map[8][16], FoodShow = 0, TarX, TarY, p = 0, LimitTime, nowDire, PausePress;
//xyi��ȫ�ֱ��������ڱ�����
char NowMap[16];
int Dire = 3;//Dire��ʾ�ߵķ����������ҷֱ���1243
int DIN = 12, CS = 11, CLK = 10, DIN2 = 13, CS2 = 7;//�ӿ�
int State = 0;//State��ʾ״̬��0������ʼ���棻1������Ϸ״̬��2����������3����ͨ��;4������ͣ
int LimitTimeBase = 6;//����( +1)��ˢ��֮������ߵ�����

LedControl lc = LedControl(DIN, CLK, CS, 4);
LedControl lc2 = LedControl(DIN2, CLK, CS2, 4);

int chars[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };
char Died[8] = { 0xE7,0x7E,0x3C,0x18,0x18,0x3C,0x7E,0xE7 };
char Suc[8] = { 0x00,0x01,0x03,0x06,0x8C,0xD8,0x70,0x20 };
char Start[8] = { 0x1E,0x20,0x20,0x3E,0x02,0x02,0x02,0x7C };
char Pause[8] = { 0x00,0x38,0x24,0x24,0x38,0x20,0x20,0x00 };
char Two[8] = { 0x00,0x7E,0x18,0x18,0x18,0x18,0x18,0x00 };

//������ʾ����  
void printByte(char* character)
{
	for (i = 0; i < 8; i++)
	{
		lc.setRow(0, i, character[i]);
	}
}

void printByte2(char* ch)
{
	for (i = 0; i < 8; i++)
	{
		lc2.setRow(0, i, ch[i]);
	}
}

void setup()
{
	pinMode(2, INPUT_PULLUP);
	//ҡ�˵�����������Ҫ
	Serial.begin(9600);
	lc.shutdown(0, false); //����ʱ��MAX72XX����ʡ��ģʽ
	lc.setIntensity(0, 2); //����������Ϊ���ʵ�ֵ(0 - 8)
	lc.clearDisplay(0); //�����ʾ
	lc2.shutdown(0, false); //����ʱ��MAX72XX����ʡ��ģʽ
	lc2.setIntensity(0, 2); //����������Ϊ���ʵ�ֵ(0 - 8)
	lc2.clearDisplay(0); //�����ʾ
}

void GetMap()
{
	for (x = 0; x < 8; x++)
	{
		NowMap[x] = 0;
		for (y = 0; y < 16; y++)
			Map[x][y] = 0;
	}
	for (x = 8; x < 16; x++)
	{
		NowMap[x] = 0;
	}
	//��ʼ��

	for (i = 0; i < BodyLen; i++)
	{
		NowMap[BodyY[i]] += chars[BodyX[i]];
		Map[BodyX[i]][BodyY[i]] = 1;
	}
}

void DrawMap()
{
	if (State == 0)
	{
		printByte(Start);
		printByte2(Two);
	}
	else if (State == 2)
	{
		printByte(Died);
		printByte2(Two);
	}
	else if (State == 3)
	{
		printByte(Suc);
		printByte2(Two);
	}
	else if (State == 4)
	{
		printByte(Pause);
		printByte2(Two);
	}
	else
	{
		GetMap();
		if (FoodShow)
		{
			FoodShow = 0;
			NowMap[FoodY] += chars[FoodX];
		}
		else
			FoodShow = 1;
		printByte(NowMap);
		printByte2(NowMap + 8);
	}
}

void GetFood()
{
	//�������ʳ��
	BodyLen -= 1;
	GetMap();
	BodyLen += 1;
	while (1)
	{
		FoodX = random(0, 7);
		FoodY = random(0, 15);
		if (Map[FoodX][FoodY] == 0 && !(TarX == FoodX && TarY == FoodY))
			break;
	}
	Map[FoodX][FoodY] = 2;
	//2����ʳ��
}

int Move()
{
	nowDire = Dire;
	TarX = BodyX[0];
	TarY = BodyY[0];
	//Serial.print("Tar: ");
	//Serial.print(TarX);
	//Serial.print(" Y: ");
	//Serial.println(TarY);
	if (nowDire == 1)
		TarY -= 1;
	else if (nowDire == 2)
		TarY += 1;
	else if (nowDire == 4)
		TarX -= 1;
	else
		TarX += 1;

	//����Ѿ�����ǽ�˻��������Լ���
	if (TarX < 0 || TarY < 0 || TarX > 7 || TarY > 15 || Map[TarX][TarY] == 1)
	{
		State = 2;
		return 1;
	}

	//����Ե��˶���
	if (FoodX == TarX && FoodY == TarY)
	{
		BodyLen += 1;
		if (BodyLen > 40)
		{
			State = 3;
			return 1;
		}
		GetFood();
	}

	for (i = BodyLen - 1; i > 0; i--)
	{
		BodyX[i] = BodyX[i - 1];
		BodyY[i] = BodyY[i - 1];
	}
	BodyX[0] = TarX;
	BodyY[0] = TarY;
	return 0;
}

void loop()
{
	LimitTime = LimitTimeBase - BodyLen / 16;
	nowx = analogRead(A1);
	nowy = analogRead(A0);
	PushZ = digitalRead(2);
	if (PushZ)
		PausePress = 0;
	else
		PausePress += 1;
	if (nowx > 768)
		PushX = 1;
	else if (nowx < 256)
		PushX = -1;
	else
		PushX = 0;
	if (nowy > 768)
		PushY = 1;
	else if (nowy < 256)
		PushY = -1;
	else
		PushY = 0;

	if (PushY > 0 && nowDire != 2)
		Dire = 1;
	else if (PushY < 0 && nowDire != 1)
		Dire = 2;
	else if (PushX > 0 && nowDire != 4)
		Dire = 3;
	else if (PushX < 0 && nowDire != 3)
		Dire = 4;

	//Serial.print("  X:");
	//Serial.print(nowx);
	//Serial.print("  Y:");
	//Serial.print(nowy);
	//Serial.print("  Z:");
	//Serial.print(nowz);
	//Serial.print("\n");
	////���ҡ�����ڵõ�����Ϣ
	//Serial.println("Initing...Now:");
	//Serial.println(State);
	//���ҡ�����ڵõ���ת��Ϊ���ֵ���Ϣ
	//ҡ����Ϣ�������
	if (State == 1)
	{
		if (PushZ == 0 && PausePress > 8)
		{
			State = 4;
			DrawMap();
			PausePress = 0;
		}
		else
		{
			//Serial.println("a:");
			//Serial.println("Initing...Now:");
			//Serial.println("Initing...Now:");
			//Serial.println("Initing...Now:");
			//Serial.println("Initing...Now:");
			//Serial.println("Initing...Now:");
			//Serial.println("Initing...Now:");
			//Serial.println("Start");
			BodyLen -= 1;
			//Serial.println("StartGetMap");
			GetMap();
			//Serial.println("HaveGotMap");
			BodyLen += 1;
			if (p >= LimitTime)
			{
				p = 0;
				Move();
			}
			else
			{
				p = p + 1;
			}
			DrawMap();
		}
	}
	else if (PushZ == 0 && State != 1)
	{
		if (State == 4 && PausePress > 8)
		{
			State = 1;
			PausePress = 0;
			DrawMap();
		}
		else if (State != 4)
		{
			State = 1;
			BodyX[0] = 4;
			BodyY[0] = 2;
			BodyX[1] = 3;
			BodyY[1] = 2;
			BodyX[2] = 2;
			BodyY[2] = 2;
			BodyLen = 3;
			//Serial.println("StartGetFood");
			TarX = 2;
			TarY = 2;
			GetFood();
			//Serial.println("HaveGotFood");
			DrawMap();
			nowDire = 3;
			Dire = 3;
			//Serial.println("InitOver!");
		}
		else
			DrawMap();
	}
	else
	{
		//Serial.println("a:");
		DrawMap();
	}
	//for (int t = 0; t < BodyLen; t++)
	//{
		//Serial.print(t);
		//Serial.print(": \\");
		//Serial.print(BodyX[t]);
		//Serial.print("  ");
		//Serial.println(BodyY[t]);
	//}
	delay(40);
}