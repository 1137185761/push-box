//编程要考虑到代码的可读性,维护性,合法性
#include<iostream>
#include<graphics.h>
#include<string>
#include<conio.h>//输入输出控制(通过键盘)

using namespace std;

#define Ration 61
#define Screen_widith 732
#define Screen_heigth 549
#define Lines 9
#define Cols  12
#define x_deviation 0
#define y_deviation 0

//宏定义可以带参数,类似于使用函数
#define isValid(p) p.lines > 0 && p.lines<Lines&&p.cols>0&&p.cols<Cols

//热键功能宏定义
#define KEY_UP 'w'
#define KEY_DOWN 's'
#define KEY_RIGHT 'a'
#define KEY_LEFT 'd'
#define KEY_QUIT 'q'

int map[Lines][Cols] = {
		{0,0,0,0,0,0,0,0,0,0,0,0},
		{0,1,0,1,1,1,1,1,1,1,0,0},
		{0,1,4,1,0,2,1,0,2,1,0,0},
		{0,1,0,1,0,1,0,0,1,1,1,0},
		{0,1,0,2,0,1,1,4,1,1,1,0},
		{0,1,1,1,0,3,1,1,1,4,1,0},
		{0,1,2,1,1,4,1,1,1,1,1,0},
		{0,1,0,0,1,0,1,1,0,0,1,0},
		{0,0,0,0,0,0,0,0,0,0,0,0}
};

enum prop//枚举类型
{
	Wall,//0
	Floor,//1
	Des,//2
	Man,//3
	Box,//4
	Hit,//5箱子命中目标
	All//6  代表的是枚举元素的总个数
};

enum con_cmd//游戏控制命令
{
	UP,
	DOWN,
	LEFT,
	RIGHT
};

struct man
{
	int lines;//小人所在的行数
	int cols;//小人所在的列数
}man_site;

IMAGE image[All];

/*************************************************
* 函数的有三个参数,分别是小人的行标和列标和要替换的道具类型
*对发生变化的变量进行合法性检查,防止越界
**************************************************/

void changeMan(int lines, int cols, enum prop cp)
{
	map[lines][cols] = cp;
	putimage(cols * Ration, lines * Ration, &image[cp]);
}

void gameCon(enum con_cmd direction)
{
	struct man next_site;
	struct man box_site;
	next_site = man_site;
	box_site = man_site;
	switch (direction)
	{
	case UP:
		next_site.lines--;
		box_site.lines -= 2;
		break;
	case DOWN:
		next_site.lines++;
		box_site.lines += 2;
		break;
	case LEFT:
		next_site.cols--;
		box_site.cols -= 2;
		break;
	case RIGHT:
		next_site.cols++;
		box_site.cols += 2;
		break;
	}
		if (isValid(next_site)&&map[next_site.lines][next_site.cols] == Floor)
		{
			changeMan(next_site.lines, next_site.cols, Man);
			changeMan(man_site.lines, man_site.cols, Floor);
			man_site = next_site;
		}
		else if (isValid(next_site) && map[next_site.lines][next_site.cols] == Box)
		{
			if (map[box_site.lines][box_site.cols] == Floor)
			{
				changeMan(box_site.lines, box_site.cols, Box);
				changeMan(next_site.lines, next_site.cols, Man);
				changeMan(man_site.lines, man_site.cols, Floor);
				man_site = next_site;
			}
			else if (map[box_site.lines][box_site.cols] == Des)
			{
				changeMan(box_site.lines, box_site.cols, Box);
				changeMan(next_site.lines, next_site.cols, Man);
				changeMan(man_site.lines, man_site.cols, Floor);
				man_site = next_site;
			}
		}
	
}

//游戏结束判断
bool isGameover()
{
	for (int i = 0; i < Lines; i++)
	{
		for (int j = 0; j < Cols; j++)
		{
			if (map[i][j] == 2)
			{
				return false;
			}
		}
	}
	return true;
}

//游戏结束场景
void gameOverSence(IMAGE *image)
{
	putimage(0, 0, image);
	settextcolor(WHITE);
	RECT rec = { 0, 0,Screen_widith,Screen_heigth };
	settextstyle(80, 0,_T("仿宋"));
	drawtext(_T("游戏结束"),&rec,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
}

int main()
{
	IMAGE bg_image;
	//加载背景
	initgraph(Screen_widith, Screen_heigth);
	loadimage(&bg_image, _T("blackground.bmp"), Screen_widith, Screen_heigth);
	putimage(0, 0, &bg_image);

	loadimage(&image[Wall], _T("wall_right.bmp"), Ration, Ration);
	loadimage(&image[Floor], _T("floor.bmp"), Ration, Ration);
	loadimage(&image[Des], _T("des.bmp"), Ration, Ration);
	loadimage(&image[Man], _T("qiezi.jpg"), Ration, Ration);
	loadimage(&image[Box], _T("box.bmp"), Ration, Ration);
	loadimage(&image[All-1], _T("box.bmp"), Ration, Ration);

	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 12; j++)
		{
			if (map[i][j] == Man)
			{
				man_site.lines = i;
				man_site.cols = j;
			}
			putimage(j * 61+ x_deviation, i* 61+ y_deviation, &image[map[i][j]]);
		}
	}

	//热键控制
	bool quit = false;
	do
	{
		if (_kbhit())//_kbhit()函数的功能:如果敲击键盘返回true;
		{
			char ch = _getch();//获取键盘输入的字符
			if (ch == 'w')
			{
				gameCon(UP);
			}
			else if (ch == 'a')
			{
				gameCon(LEFT);
			}
			else if (ch == 's')
			{
				gameCon(DOWN);
			}
			else if (ch == 'd')
			{
				gameCon(RIGHT);
			}
			else if (ch == 'q')
			{
				quit = true;
			}
			if (isGameover())
			{
				gameOverSence(&bg_image);
				system("pause");
				quit = true;
			}
		}
		Sleep(100);
	} while (quit == false);
	closegraph();
	return 0;
}