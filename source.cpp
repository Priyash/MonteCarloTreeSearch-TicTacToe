#include<iostream>
#include<cstdio>
#include<vector>
#include<string>
#include<math.h>
#include<algorithm>
#include<random>
#include<ctime>
#include<thread>
#include<chrono>

using namespace std;


class Node
{
	//No of wins
	int w;
	//no of visits;
	int v;
	Node* parent;
	vector<Node*>children;
	vector<char>board;
	char currentSymbol;
public:
	Node()
	{
		for (int i = 0; i < 9; i++)
		{
			board.push_back('-');
		}
		parent = NULL;
		w = 0;
		v = 0;
	}

	Node(vector<char>board,Node* parent)
	{
		w = v = 0;
		this->board = board;
		this->parent = parent;
	}

	vector<char>getBoard(){ return board; }
	void setCurrentSymbol(char value){ currentSymbol = value; }
	char getCurrentSymbol(){ return currentSymbol; }
	Node* getParent(){ return parent; }
	vector<Node*>getChildren(){ return children; }
	int getNoOfWins(){ return w; }
	int getNoOfVisits(){ return v; }
	bool isEndState();
	bool isBoardEmpty();
	int getNoOfValidMoves(){ return children.size(); }

	bool isLeaf();
	//for debugging
	void print_moves();
	//mone carlo methods
	Node* select();
	void expand();
	int simulate(Node* n,char opponent_symbol);
	void update(int value);
};

bool Node::isLeaf()
{
	return (children.size()==0);
}

void Node::print_moves()
{
	int c = 0;
	for (int i = 0; i < 9; i++)
	{
		cout << board[i] << " ";
		c++;
		if (c % 3 == 0)cout << endl;
	}
	cout << endl;
}


bool Node::isBoardEmpty()
{
	bool isEmpty = false;
	vector<char>temp_board = this->getBoard();
	for (int i = 0; i < 9; i++)
	{
		if (temp_board[i] == '-')
		{
			isEmpty = true;
		}
		else
			isEmpty = false;
	}

	return isEmpty;
}

bool Node::isEndState()
{
	bool endState = true;
	for (int i = 0; i < 9; i++)
	{
		if (board[i] == '-')
		{
			endState = false;
		}
	}

	return endState;
}

bool checkEndState(vector<char>board)
{
	bool end = true;
	for (int i = 0; i < 9; i++)
	{
		if (board[i] == '-')
		{
			end = false;
		}
	}
	return end;
}

bool checkWinState(vector<char>board, char symbol)
{
	int pos[8][3] = {
		{ 0, 1, 2 },
		{ 3, 4, 5 },
		{ 6, 7, 8 },
		{ 0, 3, 6 },
		{ 1, 4, 7 },
		{ 2, 5, 8 },
		{ 0, 4, 8 },
		{ 2, 4, 6 }
	};
	int c = 0;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (board[pos[i][j]] == symbol)
			{
				c++;
			}
		}
		if (c == 3)
		{
			return true;
		}
		c = 0;
	}

	return false;
}

bool checkDrawState(vector<char>board, char symbol)
{
	bool draw = false;
	if (checkEndState(board))
	{
		int pos[8][3] = {
			{ 0, 1, 2 },
			{ 3, 4, 5 },
			{ 6, 7, 8 },
			{ 0, 3, 6 },
			{ 1, 4, 7 },
			{ 2, 5, 8 },
			{ 0, 4, 8 },
			{ 2, 4, 6 }
		};
		int c = 0;
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				if (board[pos[i][j]] == symbol)
				{
					c++;
				}
			}
			if (c < 3)
			{
				draw = true;
			}
			else
			{
				draw = false;
			}
			c = 0;
		}
	}
	return draw;
}


//Monte Carlo Methods

Node* Node::select()
{
	double epsilon = 1e-6;
	double uctValue = 0.0;
	double bestValue = -numeric_limits<double>::max();
	Node* selectedChild=nullptr;
	for (auto child : children)
	{
		uctValue = (double)((child->getNoOfWins() / (child->getNoOfVisits() + epsilon)) + 2*(double)sqrt(log(getNoOfVisits() + 1) / (child->getNoOfVisits() + epsilon)));
		if (bestValue < uctValue)
		{
			bestValue = uctValue;
			selectedChild = child;
		}
	}
	return selectedChild;
}

void Node::expand()
{
	if (!this->isLeaf())return;
	vector<char>t_board = this->getBoard();
	for (int i = 0; i < 9; i++)
	{
		if (t_board[i] == '-')
		{
			t_board[i] = this->getCurrentSymbol();
			Node* temp = new Node(t_board,this);
			temp->setCurrentSymbol(this->getCurrentSymbol());
			children.push_back(temp);
			t_board[i] = '-';
		}
	}
}

int Node::simulate(Node* n,char opponent_symbol)
{
	vector<char>board_t = n->getBoard();
	srand(time(0));
	bool myTurn = false;
	while (!checkEndState(board_t))
	{		
		if (myTurn)
		{
			int pos = rand() % 9;
			while (board_t[pos] != '-')
			{
				pos = rand() % 9;
			}
			board_t[pos] = currentSymbol;
			if (checkWinState(board_t, currentSymbol))return 1;
			myTurn = false;
		}
		else if (!myTurn)
		{
			int pos = rand() % 9;
			while (board_t[pos] != '-')
			{
				pos = rand() % 9;
			}
			board_t[pos] = opponent_symbol;
			if (checkWinState(board_t, opponent_symbol))return -1;
			myTurn = true;
		}
		
	}
	return 0;
}

void Node::update(int value)
{
	this->v++;
	this->w += value;
}


//-----------------------------------------------------------PLAYER CLASS-------------------------------------//
//player class that will execute monte carlo framework 
class Player
{
	vector<char>board;
	bool isMyTurn = false;
	char currentSymbol;
	char opponentSymbol;
public:
	Player(vector<char>board, bool isMyTurn, char currentSymbol, char opponentSymbol)
	{
		this->board = board;
		this->isMyTurn = isMyTurn;
		this->currentSymbol = currentSymbol;
		this->opponentSymbol = opponentSymbol;
	}

	vector<char>getBoard(){ return board; }
	char getCurrentSymbol(){ return currentSymbol; }
	char getOpponentSymbol(){ return opponentSymbol; }
	void play();
};
//Framework for select ,expand ,playout and update these four task will repeat to the given number of time
void Player::play()
{
	int value;
	Node* temp = NULL;
	Node* root = new Node(board, NULL);
	for (int i = 0; i < 10000; i++)
	{
		Node* curr = root;
		//currentSymbol ->the one this player will play with
		curr->setCurrentSymbol(currentSymbol);
		while (!curr->isLeaf())
		{
			curr = curr->select();
		}

		//Be careful this is one heck of a choke (alorithm says expand the node if it is leaf node and not a end game state
		//what if  it is a leaf node and end game state so we don't need to simulate so instead have to check the winnin or losing 
		//state of the end state which is a leaf node and based on that move to the update rule in monte carlo tree search
		if (curr->isLeaf() && curr->isEndState())
		{
			if (checkWinState(curr->getBoard(), currentSymbol))
			{
				value = 1;
			}
			else if (checkWinState(curr->getBoard(), opponentSymbol))
			{
				value = -1;
			}
			else
			{
				value = 0;
			}
			temp = curr;
		}
		else
		{
			curr->expand();
			Node* node = curr->select();
			value = curr->simulate(node, opponentSymbol);
			temp = node;
		}
		while (temp->getParent() != NULL)
		{
			temp->update(value);
			temp->getParent()->update(value);
			temp = temp->getParent();
		}

	}
	vector<Node*>children = root->getChildren();
	auto itr = max_element(children.begin(), children.end(), [](Node* n1, Node* n2)->bool{return n1->getNoOfVisits() < n2->getNoOfVisits(); });
	Node* best = *itr;
	best->print_moves();
	board = best->getBoard();
	isMyTurn = false;
}

//-----------------------------------------------------------------------------------------------------------//

int bot1_w = 0;
int bot2_w = 0;
void Game()
{
	Node* root = new Node();
	vector<char>board = root->getBoard();
	while (!checkEndState(board))
	{
		Player* bot1 = new Player(board, true, 'x', 'o');
		bot1->play();
		board = bot1->getBoard();
		if (checkWinState(board, bot1->getCurrentSymbol()))
		{
			cout << "Bot 1 wins!!!" << endl;
			bot1_w++;
			break;
		}
		if (checkDrawState(board, bot1->getCurrentSymbol()))
		{
			cout << "Draw" << endl;
			break;
		}
		Player* bot2 = new Player(board, true, 'o', 'x');
		bot2->play();
		board = bot2->getBoard();
		if (checkWinState(board, bot2->getCurrentSymbol()))
		{
			cout << "Bot 2 wins!!!" << endl;
			bot2_w;
			break;
		}
		if (checkDrawState(board, bot2->getCurrentSymbol()))
		{
			cout << "Draw" << endl;
			break;
		}
		//delay 2 secs;
		this_thread::sleep_for(chrono::seconds(3));
	}
}


int main()
{

	//PlayGame();
	Game();
	return 0;
}
