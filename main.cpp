#include <stdio.h>
#include <iostream>
#include <vector>
#include <limits.h>
#include <array>
#include <sstream>
#include <fstream>
#include <string>

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

using namespace std;

unsigned int numCol = 6;
unsigned int numRow = 7;
unsigned int player = 1;
unsigned int computer = 2;
unsigned int maxDepth = 8;

bool gameOver = false;
unsigned int turns = 0;
unsigned int currentplayer = player;

vector<vector<int>> board(numRow, vector<int>(numCol));

void saveStatus(int difficulty, const vector<vector<int>>& board)
{
    ofstream file("gameState.csv");

    file << "Difficulty," << difficulty << endl;

    file << "Board" << endl;

    for (const auto& fila : board)
    {
        for (int valor : fila)
        {
            file << valor << ",";
        }
        file << endl;
    }
    file.close();
}

void cargarEstado(int& difficulty, vector<vector<int>>& board)
{
    ifstream file("gameState.csv");

    if (!file.is_open())
    {
        cout << "The status file could not be opened" << endl;
        return;
    }

    string line, token;

    getline(file, line);
    stringstream ss(line);
    getline(ss, token, ',');
    getline(ss, token);
    difficulty = stoi(token);

    getline(file, line);
    board.clear();

    while (getline(file, line))
    {
        stringstream ss(line);
        vector<int> fila;

        while (getline(ss, token, ','))
        {
            fila.push_back(stoi(token));
        }
        board.push_back(fila);
    }
    file.close();
}

void initBoard()
{
	for (unsigned int r = 0; r < numRow; r++)
    {
		for (unsigned int c = 0; c < numCol; c++)
        {
			board[r][c] = 0;
		}
	}
}

vector<vector<int> > copyBoard(vector<vector<int> > b)
{
	vector<vector<int>> newBoard(numRow, vector<int>(numCol));
	for (unsigned int r = 0; r < numRow; r++)
    {
		for (unsigned int c = 0; c < numCol; c++)
        {
			newBoard[r][c] = b[r][c];
		}
	}
	return newBoard;
}

void printBoard(vector<vector<int> > &b)
{
	for (unsigned int r = 0; r < numRow; r++)
    {
		for (unsigned int c = 0; c < numCol; c++)
        {
			switch (b[numRow - r - 1][c])
            {
			    case 0: cout << " ' "; break;
			    case 1: cout << " O "; break;
			    case 2: cout << " X "; break;
			}
		}
		cout << endl;
	}

    for (unsigned int i = 0; i < numCol; i++)
    {
		cout << "[" << (i+1) << "]";
	}

	cout << endl;
}

void makeMove(vector<vector<int>> &board, int j, unsigned int piece)
{
	for (unsigned int i = 0; i < numRow; i++)
    {
		if (board[i][j] == 0)
        {
			board[i][j] = piece;
			break;
		}
	}
}

bool winningMove(vector<vector<int> > &b, unsigned int p)
{
	unsigned int winSequence = 0;

	for (unsigned int c = 0; c < numCol - 3; c++)
    {
		for (unsigned int r = 0; r < numRow; r++)
        {
			for (int i = 0; i < 4; i++)
            {
				if ((unsigned int)b[r][c + i] == p)
                {
					winSequence++;
				}
				if (winSequence == 4)
                {
                    return true;
                }
			}
			winSequence = 0;
		}
	}

	for (unsigned int c = 0; c < numCol; c++)
    {
		for (unsigned int r = 0; r < numRow - 3; r++)
        {
			for (int i = 0; i < 4; i++)
            {
				if ((unsigned int)b[r + i][c] == p)
                {
					winSequence++;
				}
				if (winSequence == 4)
                {
                    return true;
                }
			}
			winSequence = 0;
		}
	}

	for (unsigned int c = 0; c < numCol - 3; c++)
    {
		for (unsigned int r = 3; r < numRow; r++)
        {
			for (int i = 0; i < 4; i++) {
				if ((unsigned int)b[r - i][c + i] == p)
                {
					winSequence++;
				}
				if (winSequence == 4)
                {
                    return true;
                }
			}
			winSequence = 0;
		}
	}

	for (unsigned int c = 0; c < numCol - 3; c++)
    {
		for (unsigned int r = 0; r < numRow - 3; r++)
        {
			for (int i = 0; i < 4; i++) {
				if ((unsigned int)b[r + i][c + i] == p)
                {
					winSequence++;
				}
				if (winSequence == 4)
                {
                    return true;
                }
			}
			winSequence = 0;
		}
	}
	return false;
}

int heurFunction(unsigned int g, unsigned int b, unsigned int z)
{
	int score = 0;
	if (g == 4) { score += 500001; }
	else if (g == 3 && z == 1) { score += 5000; }
	else if (g == 2 && z == 2) { score += 500; }
	else if (b == 2 && z == 2) { score -= 501; }
	else if (b == 3 && z == 1) { score -= 5001; }
	else if (b == 4) { score -= 500000; }
	return score;
}

int scoreSet(vector<unsigned int> v, unsigned int p)
{
	unsigned int good = 0;
	unsigned int bad = 0;
	unsigned int empty = 0;
	for (unsigned int i = 0; i < v.size(); i++)
    {
		good += (v[i] == p) ? 1 : 0;
		bad += (v[i] == player || v[i] == computer) ? 1 : 0;
		empty += (v[i] == 0) ? 1 : 0;
	}

	bad -= good;
	return heurFunction(good, bad, empty);
}

int tabScore(vector<vector<int>> b, unsigned int p)
{
	int score = 0;
	vector<unsigned int> rs(numCol);
	vector<unsigned int> cs(numRow);
	vector<unsigned int> set(4);
	
	for (unsigned int r = 0; r < numRow; r++)
    {
		for (unsigned int c = 0; c < numCol; c++)
        {
			rs[c] = b[r][c];
		}
		for (unsigned int c = 0; c < numCol - 3; c++)
        {
			for (int i = 0; i < 4; i++)
            {
				set[i] = rs[c + i];
			}
			score += scoreSet(set, p);
		}
	}

	for (unsigned int c = 0; c < numCol; c++)
    {
		for (unsigned int r = 0; r < numRow; r++)
        {
			cs[r] = b[r][c];
		}
		for (unsigned int r = 0; r < numRow - 3; r++)
        {
			for (int i = 0; i < 4; i++)
            {
				set[i] = cs[r + i];
			}
			score += scoreSet(set, p);
		}
	}

	for (unsigned int r = 0; r < numRow - 3; r++)
    {
		for (unsigned int c = 0; c < numCol; c++)
        {
			rs[c] = b[r][c];
		}
		for (unsigned int c = 0; c < numCol - 3; c++)
        {
			for (int i = 0; i < 4; i++)
            {
				set[i] = b[r + i][c + i];
			}
			score += scoreSet(set, p);
		}
	}

	for (unsigned int r = 0; r < numRow - 3; r++)
    {
		for (unsigned int c = 0; c < numCol; c++)
        {
			rs[c] = b[r][c];
		}
		for (unsigned int c = 0; c < numCol - 3; c++)
        {
			for (int i = 0; i < 4; i++)
            {
				set[i] = b[r + 3 - i][c + i];
			}
			score += scoreSet(set, p);
		}
	}
	return score;
}

array<int, 2> miniMax(vector<vector<int>> &b, unsigned int d, int alf, int bet, unsigned int p)
{
	if (d == 0 || d >= (numCol * numRow) - turns)
    {
		return array<int, 2>{tabScore(b, computer), -1};
	}
	if (p == computer)
    {
		array<int, 2> moveSoFar = {INT_MIN, -1};
		if (winningMove(b, player))
        {
			return moveSoFar;
		}

		for (unsigned int c = 0; c < numCol; c++)
        {
			if (b[numRow - 1][c] == 0)
            {
				vector<vector<int> > newBoard = copyBoard(b);
				makeMove(newBoard, c, p);
				int score = miniMax(newBoard, d - 1, alf, bet, player)[0];
				if (score > moveSoFar[0])
                {
					moveSoFar = {score, (int)c};
				}
				alf = max(alf, moveSoFar[0]);
				if (alf >= bet)
                {
                    break;
                }
			}
		}
		return moveSoFar;
	}

	else
    {
		array<int, 2> moveSoFar = {INT_MAX, -1};
		if (winningMove(b, computer))
        {
			return moveSoFar;
		}
		for (unsigned int c = 0; c < numCol; c++)
        {
			if (b[numRow - 1][c] == 0)
            {
				vector<vector<int> > newBoard = copyBoard(b);
				makeMove(newBoard, c, p);
				int score = miniMax(newBoard, d - 1, alf, bet, computer)[0];

				if (score < moveSoFar[0])
                {
					moveSoFar = {score, (int)c};
				}
				bet = min(bet, moveSoFar[0]);
				if (alf >= bet)
                {
                    break;
                }
			}
		}
		return moveSoFar;
	}
}

int aiMove(int d)
{
    if (d == 1) {maxDepth = 3;}
    if (d == 2) {maxDepth = 5;}
    if (d == 3) {maxDepth = 8;}

	return miniMax(board, maxDepth, 0 - INT_MAX, INT_MAX, computer)[1];
}

int userMove(int d)
{
	int move = -2;

	while (true)
    {
		cout << "Enter a column [0 to save]: ";
		cin >> move;
        move--;

		if (!cin)
        {
			cin.clear();
			cin.ignore(INT_MAX, '\n');
			cout << "Use a value [1-" << numCol << "]" << endl;
		}
        else if (move == -1)
        {
            saveStatus(d, board);
            cout << endl;
            exit(1);
        }
		else if (!((unsigned int) move < numCol && move >= 0))
        {
			cout << "That is not a valid column." << endl;
		}
		else if (board[numRow - 1][move] != 0)
        {
			cout << "That column is full." << endl;
		}
		else
        {
			break;
		}
		cout << endl;
	}
	return move;
}

void playGame(int d)
{
    if (d == 1) {cout << "Difficulty: Easy" << endl;}
    if (d == 2) {cout << "Difficulty: Medium" << endl;}
    if (d == 3) {cout << "Difficulty: Hard" << endl;}
    cout << endl;

	printBoard(board);

	while (!gameOver)
    {
		if (currentplayer == computer)
        {
			makeMove(board, aiMove(d), computer);
		}

		else if (currentplayer == player)
        {
			makeMove(board, userMove(d), player);
		}
		else if (turns == numRow * numCol)
        {
			gameOver = true;
		}

		gameOver = winningMove(board, currentplayer);
		currentplayer = (currentplayer == 1) ? 2 : 1;
		turns++;
		cout << endl;
		printBoard(board);
	}

	if (turns == numRow * numCol)
    {
		cout << "Draw!" << endl;
	}
	else
    {
		cout << ((currentplayer == player) ? "AI Wins!" : "player Wins!") << endl;
	}
}
 
int setDifficulty()
{
    int option = 0;

    while (true)
    {
        cout << "==Submenu==" << endl;
        cout << "1. Easy" << endl;
        cout << "2. Medium" << endl;
        cout << "3. Hard" << endl;
        cout << "Choose a option: ";
        cin >> option;

        if (option != 1 && option != 2 && option != 3)
        {
            cout << endl;
            cout << "Invalid option, try again" << endl;
            cout << endl;
        }
        else
        {
            return option;
        }
    }
}

int main(int argc, char** argv)
{
	int i = -1; bool flag = false;

	if (argc == 2)
    {
		istringstream in(argv[1]);
		if (!(in >> i))
        {
            flag = true;
        }
		if (i > (int)(numRow * numCol) || i <= -1)
        {
            flag = true;
        }
		if (flag)
        {
            cout << "Invalid command line argument, using default depth = 5." << endl;
        }
		else
        {
            maxDepth = i;
        }
	}

	initBoard();

    int option;
    int difficulty = 2;

    do
    {
        cout << "==Menu==" << endl;
        cout << "1. New Game" << endl;
        cout << "2. Load" << endl;
        cout << "3. Change difficulty" << endl;
        cout << "4. Exit" << endl;
        cout << "Choose a option: ";
        cin >> option;
        cout << endl;

        switch (option)
        {
            case 1:
                playGame(difficulty);
                break;
            
            case 2:
                cargarEstado(difficulty, board);
                break;

            case 3:
                difficulty = setDifficulty();
                break;

            case 4:
                cout << "Closing program" << endl;
                break;

            default:
                cout << "Invalid option, try again" << endl;
        }
        cout << endl;

    } while (option != 4);

	return 0;
}