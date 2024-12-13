#include <GL/glut.h>
#include <stdlib.h>
#include <time.h>
#include <queue>
#include <vector>
#include <cstdlib>
#include <windows.h>

using namespace std;

const int mazeWidth = 25;
const int mazeHeight = 25;
int windowWidth = 500;
int windowHeight = 500;
float offsetX, offsetY;
int cellSize;

int maze[mazeHeight][mazeWidth] = {};

int mode = 0;
bool isMousePressed = false; // �l�ܷƹ����U���A
int lastMouseRow = -1;
int lastMouseCol = -1;

int agentStartX = 1;
int agentStartY = 1;

bool visited[mazeHeight][mazeWidth] = {}; // �����O�_���L
bool searchCompleted = false;

struct Node {
    int row, col;
    std::vector<std::pair<int, int>> path; // �x�s��e��m
};

void drawPath() {
    for (int i = 0; i < mazeHeight; ++i) {
        for (int j = 0; j < mazeWidth; ++j) {
            if (maze[i][j] == 2) {
                int x = j * cellSize + offsetX;
                int y = (mazeHeight - 1 - i) * cellSize + offsetY;
                glColor3f(0.0, 1.0, 0.0);
                glBegin(GL_QUADS);
                glVertex2i(x, y);
                glVertex2i(x + cellSize, y);
                glVertex2i(x + cellSize, y + cellSize);
                glVertex2i(x, y + cellSize);
                glEnd();
            }
        }
    }
    glFlush();
}

void generateMaze() {
    for (int i = 0; i < mazeHeight; ++i) {
        for (int j = 0; j < mazeWidth; ++j) {
            if (i == 0 || i == mazeHeight - 1 || j == 0 || j == mazeWidth - 1) {
                maze[i][j] = 1; // ��ɩT�w�����
            } else {
                maze[i][j] = (rand() % 100 < 30) ? 1 : 0; // 30% ���v�����
            }
        }
    }
}

void init() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    cellSize = windowWidth / mazeWidth < windowHeight / mazeHeight ? windowWidth / mazeWidth : windowHeight / mazeHeight;

    offsetX = (windowWidth - mazeWidth * cellSize) / 2.0f;
    offsetY = (windowHeight - mazeHeight * cellSize) / 2.0f;

    gluOrtho2D(0.0, windowWidth, 0.0, windowHeight);

    srand(time(NULL));
    generateMaze();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    windowWidth = w;
    windowHeight = h;

    // ���s�p���l�j�p�A�ϰg�c�A������
    cellSize = windowWidth / mazeWidth < windowHeight / mazeHeight ? windowWidth / mazeWidth : windowHeight / mazeHeight;

    // �w�d�Ŷ��A���g�c���B�\������r
    int leftMargin = 120; // �w�d 120 �������Ŷ�
    offsetX = leftMargin + (windowWidth - leftMargin - mazeWidth * cellSize) / 2.0f;
    offsetY = (windowHeight - mazeHeight * cellSize) / 2.0f;

    gluOrtho2D(0.0, windowWidth, 0.0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
}

void drawCell(int row, int col) {
    int x = col * cellSize + offsetX;
    int y = (mazeHeight - 1 - row) * cellSize + offsetY;

    // �ھڮ�l�����]�m�C��
    if (maze[row][col] == 3) {
        glColor3f(0.0, 0.0, 1.0); // �Ŧ��ܥ��b�X�ݪ��`�I
    }
    else if (maze[row][col] == 2) {
        glColor3f(0.0, 1.0, 0.0); // ����ܥ��T���|
    }
    else if (row == mazeHeight - 2 && col == mazeWidth - 2) {
        glColor3f(0.0, 1.0, 0.0); // �����ܲ��I
    } else if (row == agentStartY && col == agentStartX) {
        glColor3f(1.0, 0.0, 0.0); // �ѹ���m
    } else if (maze[row][col] == 1) {
        glColor3f(0.0, 0.0, 0.0); // �¦����
    } else {
        glColor3f(0.8, 0.8, 0.8); // �Ǧ�q�D
    }

    // ø�s���
    glBegin(GL_QUADS);
    glVertex2i(x, y);
    glVertex2i(x + cellSize, y);
    glVertex2i(x + cellSize, y + cellSize);
    glVertex2i(x, y + cellSize);
    glEnd();
}

void drawMaze() {
    for (int i = 0; i < mazeHeight; ++i) {
        for (int j = 0; j < mazeWidth; ++j) {
            drawCell(i, j);
        }
    }
}

void drawText(const char* text, int x, int y) {

    glColor3f(0.0, 0.0, 0.0); // �¦��r
    glRasterPos2i(x, y);

    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

void setAgentStart(int x, int y) {
    int col = (x - offsetX) / cellSize;
    int row = (y - offsetY) / cellSize;

    if (row >= 0 && row < mazeHeight && col >= 0 && col < mazeWidth) {
        agentStartX = col;
        agentStartY = row;
    }
}

void toggleWall(int x, int y) {
    int col = (x - offsetX) / cellSize;
    int row = (windowHeight - y - offsetY) / cellSize;
    // �p�G�O��ɩβ��I�A�������L
    if (row <= 0 || row >= mazeHeight - 1 || col <= 0 || col >= mazeWidth - 1 ||
        (row == mazeHeight - 1 && col == mazeWidth - 1)) {
        return;
    }
    maze[row][col] = (maze[row][col] == 1) ? 0 : 1; // ����������A
    drawCell(row, col); // �u��s��ʪ���l
}

void mouseDrag(int x, int y) {
    int col = (x - offsetX) / cellSize;
    int row = (y - offsetY) / cellSize;
    if (isMousePressed && (row != lastMouseRow || col != lastMouseCol)) {
        toggleWall(x, windowHeight - y); // �����즲��m���
        glFlush();
        lastMouseRow = row;
        lastMouseCol = col;
    }
}

void menu(int item) {
    switch (item) {
        case 0:
            for (int i = 0; i < mazeHeight; ++i) {
                for (int j = 0; j < mazeWidth; ++j) {
                    if (maze[i][j] == 2) {
                        maze[i][j] = 0;
                    }
                    if (maze[i][j] == 3) {
                        maze[i][j] = 0;
                    }
                }
            }
            searchCompleted = false;
            glutPostRedisplay();
            mode = 0;
            break;
        case 1:
            for (int i = 0; i < mazeHeight; ++i) {
                for (int j = 0; j < mazeWidth; ++j) {
                    if (maze[i][j] == 2) {
                        maze[i][j] = 0;
                    }
                    if (maze[i][j] == 3) {
                        maze[i][j] = 0;
                    }
                }
            }
            searchCompleted = false;
            glutPostRedisplay();
            mode = 1;
            break;
        case 2:
            for (int i = 0; i < mazeHeight; ++i) {
                for (int j = 0; j < mazeWidth; ++j) {
                    if (maze[i][j] == 2) {
                        maze[i][j] = 0;
                    }
                    if (maze[i][j] == 3) {
                        maze[i][j] = 0;
                    }
                }
            }
            searchCompleted = false;
            mode = 2;
            break;
    }
    glutPostRedisplay();
}

void drawButton(const char* text, int x, int y, int width, int height) {
    // ø�s���s�I��
    glColor3f(0.7, 0.7, 0.7); // �Ǧ�I��
    glBegin(GL_QUADS);
    glVertex2i(x, y);
    glVertex2i(x + width, y);
    glVertex2i(x + width, y + height);
    glVertex2i(x, y + height);
    glEnd();

    // ø�s���s���
    glColor3f(0.0, 0.0, 0.0); // �¦����
    glLineWidth(2.0);
    glBegin(GL_LINE_LOOP);
    glVertex2i(x, y);
    glVertex2i(x + width, y);
    glVertex2i(x + width, y + height);
    glVertex2i(x, y + height);
    glEnd();

    // �b���s������ܤ�r
    glColor3f(0.0, 0.0, 0.0); // �¦��r
    glRasterPos2i(x + width / 4, y + height / 3);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        text++;
    }
}

bool findingPath_BFS(int startX, int startY) {
    memset(visited, false, sizeof(visited));  // �C���j�M�e���m visited
    std::queue<Node> q;
    q.push({startY, startX, {{startY, startX}}});
    visited[startY][startX] = true;

    // �W�U���k�|�Ӥ�V
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};

    bool pathFound = false;

    while (!q.empty()) {
        if (searchCompleted) {
            break;
        }
        Node current = q.front();
        q.pop();

        // �p�G�����I�A�аO���|�ð���
        if (current.row == mazeHeight - 2 && current.col == mazeWidth - 2) {
            for (auto& p : current.path) {
                maze[p.first][p.second] = 2; // �аO�����T���|
                drawCell(p.first, p.second);
                glFlush();
                Sleep(20); // �Ȱ��H�[��
            }
            pathFound = true;
            searchCompleted = true;

            break; // �������X�j��A�קK�A�j�M
        }

        // ��s��e�`�Iø��
        if (!searchCompleted && maze[current.row][current.col] == 0 &&
            !(current.row == startY && current.col == startX)) {
            maze[current.row][current.col] = 3; // �ȮɼаO�����b�X�ݪ���l
            drawCell(current.row, current.col); // ���sø�s��Ӱg�c
            glFlush(); // ��s�e��
            Sleep(25);
        }

        // �аO�U�@�h�`�I
        for (int i = 0; i < 4; ++i) {
            int newRow = current.row + dy[i];
            int newCol = current.col + dx[i];

            if (newRow >= 0 && newRow < mazeHeight && newCol >= 0 && newCol < mazeWidth &&
                maze[newRow][newCol] == 0 && !visited[newRow][newCol]) {
                visited[newRow][newCol] = true;

                std::vector<std::pair<int, int>> newPath = current.path;
                newPath.push_back({newRow, newCol});
                q.push({newRow, newCol, newPath});
            }
        }
    }

    if (searchCompleted) {
        for (int i = 0; i < mazeHeight; ++i) {
            for (int j = 0; j < mazeWidth; ++j) {
                if (maze[i][j] == 3) {
                    maze[i][j] = 0;
                }
            }
        }
    }

    return pathFound;
}


void mouseClick(int button, int state, int x, int y) {

    if (button == GLUT_LEFT_BUTTON) {
        if(mode == 0){
            if (state == GLUT_DOWN) {
                isMousePressed = true;
                lastMouseCol = (x - offsetX) / cellSize;
                lastMouseRow = (y - offsetY) / cellSize;
                toggleWall(x, windowHeight - y); // �����I����m���
            } else if (state == GLUT_UP) {
                isMousePressed = false;
            }
            glFlush(); // �T�O��ӧ�s�ߧY��s
        }
        else if(mode == 1){
            setAgentStart(x,y);
        }
        else if (mode == 2 && (x >= 10 && x <= 90 && windowHeight - y >= 10 && windowHeight - y <= 40)) {
            if(searchCompleted) return;
            searchCompleted = false;
            // mode 2 �B ���s��m�Q���U
            if (findingPath_BFS(agentStartX, agentStartY)){
            } else {
                drawText("No path found!", 10, windowHeight - 70);
            }
            glFlush();
        }
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawMaze();
    glPushMatrix();
    glLoadIdentity();
    drawText("411262075", 10, windowHeight - 30);
    drawText("Sheng-Zhe Cheng", 8, windowHeight - 50);
    if(mode == 2){
        drawButton("Start", 10, 10, 80, 30);
    }
    glFlush(); // ��w�Ĩ�s
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); // �ϥγ�w�ļҦ�
    glutInitWindowSize(800, 400);
    glutCreateWindow("Maze Outline");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseClick);
    glutMotionFunc(mouseDrag);

    glutCreateMenu(menu);
    glutAddMenuEntry("setWall", 0);
    glutAddMenuEntry("setMousePosition", 1);
    glutAddMenuEntry("findingPath",2);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMainLoop();

    return 0;
}

