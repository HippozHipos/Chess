#define OLC_PGE_APPLICATION
#pragma warning(push)
#pragma warning(disable: 26812) 
#pragma warning(disable: 26451) 
#include "pixelGameEngine.h"
#pragma warning(pop)

struct PieceString
{
	static const std::string pawn;
	static const std::string king;
	static const std::string rook;
	static const std::string queen;
	static const std::string knight;
	static const std::string bishop;
};

const std::string PieceString::pawn = "Pawn";
const std::string PieceString::king = "King";
const std::string PieceString::rook = "Rook";
const std::string PieceString::queen = "Queen";
const std::string PieceString::knight = "Knight";
const std::string PieceString::bishop = "Bishop";

struct Board
{
public:
	Board() {}
	Board(olc::vi2d screenSize, olc::vi2d nSquares) :
		nSquares{ nSquares },
		squareSize { screenSize / nSquares }
	{
	}

	olc::vi2d nSquares{};
	olc::vf2d squareSize{};

};

struct MovementValidator;
class Piece
{
public:
	enum class Type
	{
		KING,
		QUEEN,
		ROOK,
		KNIGHT,
		PAWN,
		BISHOP
	};

	enum class Color
	{
		BLACK,
		WHITE
	};

public:
	Piece(const olc::vf2d& position, Color color, MovementValidator* validator);

	virtual ~Piece();

public:
	olc::vf2d position;
	virtual Type GetType() const = 0;

	Color GetColor() const
	{
		return color;
	}

	std::vector<olc::vi2d> GetValidSquares(std::vector<Piece*> pieces, const olc::vf2d& position, const Board& board) const;

	bool operator==(const Piece& other)
	{
		return this == &other;
	}

private:
	Color color;

protected:
	MovementValidator* movementValidator;

};

class Pawn : public Piece
{
public:
	Pawn(const olc::vf2d& position, Color color);

public:
	Type GetType() const override
	{
		return Type::PAWN;
	}
};

class King : public Piece
{
public:
	King(const olc::vf2d& position, Color color);

public:
	Type GetType() const override
	{
		return Type::KING;
	}
};

class Rook : public Piece
{
public:
	Rook(const olc::vf2d& position, Color color);

public:
	Type GetType() const override
	{
		return Type::ROOK;
	}
};

class Queen : public Piece
{
public:
	Queen(const olc::vf2d& position, Color color);

public:
	Type GetType() const override
	{
		return Type::QUEEN;
	}
};

class Bishop : public Piece
{
public:
	Bishop(const olc::vf2d& position, Color color);

public:
	Type GetType() const override
	{
		return Type::BISHOP;
	}
};

class Knight : public Piece
{
public:
	Knight(const olc::vf2d& position, Color color);

public:
	Type GetType() const override
	{
		return Type::KNIGHT;
	}
};

olc::vi2d screenToSquare(const olc::vf2d& screen, const Board& board)
{
	return (olc::vi2d)(screen / board.squareSize);
}

olc::vf2d squareToScreen(const olc::vi2d square, const Board& board)
{
	return (olc::vf2d)(square * board.squareSize);
}

Piece* getPieceInSquare(const olc::vi2d& square, const std::vector<Piece*> pieces, const Board& board)
{
	for (auto& piece : pieces)
	{
		if (screenToSquare(piece->position, board) == square)
		{
			return piece;
		}
	}
	return nullptr;
}

struct MovementValidator
{
	virtual std::vector<olc::vi2d> GetValidSquares(const Piece&, const olc::vf2d&, const Board&, std::vector<Piece*>&) = 0;
	std::vector<olc::vi2d> GetOccupiableSquares(std::vector<Piece*>& pieces, const olc::vf2d& position, const Piece& pawn, const Board& board)
	{
		std::vector<olc::vi2d> validSquares = GetValidSquares(pawn, position, board, pieces);
		for (auto& piece : pieces)
		{
			if (!(*piece == pawn))
			{
				for (auto& validSquare : validSquares)
				{
					if (screenToSquare(piece->position, board) == validSquare)
					{
						validSquares.erase(std::remove(validSquares.begin(), validSquares.end(), validSquare), validSquares.end());
					}

					//std::cout << validSquare << '\n';
					if (validSquare.x < 0 || validSquare.x > 7 || validSquare.y < 0 || validSquare.y > 7)
					{
						validSquares.erase(std::remove(validSquares.begin(), validSquares.end(), validSquare), validSquares.end());
					}
				}
			}
		}
		return validSquares;
	}
	virtual ~MovementValidator() = default;
};

struct PawnMovementValidator : public MovementValidator
{
	std::vector<olc::vi2d> GetValidSquares(const Piece& pawn, const olc::vf2d& position, const Board& board, std::vector<Piece*>&) override
	{
		std::vector<olc::vi2d> outValidSquares;
		if (pawn.GetColor() == Piece::Color::BLACK)
		{
			outValidSquares.push_back(screenToSquare(position, board) + olc::vi2d{ 0, 1 });
		}
		else
		{
			outValidSquares.push_back(screenToSquare(position, board) + olc::vi2d{ 0, -1 });
		}

		return outValidSquares;
	}
};

struct KingMovementValidator : public MovementValidator
{
	std::vector<olc::vi2d> GetValidSquares(const Piece&, const olc::vf2d& position, const Board& board, std::vector<Piece*>&) override
	{
		std::vector<olc::vi2d> outValidSquares;

		outValidSquares.push_back(screenToSquare(position, board) + olc::vi2d{ 0, 1 });
		outValidSquares.push_back(screenToSquare(position, board) + olc::vi2d{ 0, -1 });
		outValidSquares.push_back(screenToSquare(position, board) + olc::vi2d{ 1, 0 });
		outValidSquares.push_back(screenToSquare(position, board) + olc::vi2d{ -1, 0 });

		return outValidSquares;
	}
};

struct QueenMovementValidator : public MovementValidator
{
	std::vector<olc::vi2d> GetValidSquares(const Piece& queen, const olc::vf2d& position, const Board& board, std::vector<Piece*>& pieces) override
	{
		std::vector<olc::vi2d> outValidSquares;

		auto addSquare = [&](olc::vi2d dir)
		{
			olc::vi2d current = screenToSquare(position, board);
			for (int i = 0; i < 8; i++)
			{
				current += dir;
				for (auto& piece : pieces)
				{
					if (!(*piece == queen))
					{
						if (current == screenToSquare(piece->position, board))
							return;
					}
				}
				outValidSquares.push_back(current);
			}
		};

		addSquare({ 1, 1 }); addSquare({ -1, -1 });
		addSquare({ 1, -1 }); addSquare({ -1, 1 });
		addSquare({ 0, -1 }); addSquare({ 0, 1 });
		addSquare({ 1, 0 }); addSquare({ -1, 0 });
		return outValidSquares;
	}
};

struct RookMovementValidator : public MovementValidator
{
	std::vector<olc::vi2d> GetValidSquares(const Piece& queen, const olc::vf2d& position, const Board& board, std::vector<Piece*>& pieces) override
	{
		std::vector<olc::vi2d> outValidSquares;

		auto addSquare = [&](olc::vi2d dir)
		{
			olc::vi2d current = screenToSquare(position, board);
			for (int i = 0; i < 8; i++)
			{
				current += dir;
				for (auto& piece : pieces)
				{
					if (!(*piece == queen))
					{
						if (current == screenToSquare(piece->position, board))
							return;
					}
				}
				outValidSquares.push_back(current);
			}
		};

		addSquare({ 0, -1 }); addSquare({ 0, 1 });
		addSquare({ 1, 0 }); addSquare({ -1, 0 });
		return outValidSquares;
	}
};

struct BishopMovementValidator : public MovementValidator
{
	std::vector<olc::vi2d> GetValidSquares(const Piece& queen, const olc::vf2d& position, const Board& board, std::vector<Piece*>& pieces) override
	{
		std::vector<olc::vi2d> outValidSquares;

		auto addSquare = [&](olc::vi2d dir)
		{
			olc::vi2d current = screenToSquare(position, board);
			for (int i = 0; i < 8; i++)
			{
				current += dir;
				for (auto& piece : pieces)
				{
					if (!(*piece == queen))
					{
						if (current == screenToSquare(piece->position, board))
							return;
					}
				}
				outValidSquares.push_back(current);
			}
		};

		addSquare({ 1, 1 }); addSquare({ -1, -1 });
		addSquare({ 1, -1 }); addSquare({ -1, 1 });
		return outValidSquares;
	}
};

struct KnightMovementValidator : public MovementValidator
{
	std::vector<olc::vi2d> GetValidSquares(const Piece& queen, const olc::vf2d& position, const Board& board, std::vector<Piece*>& pieces) override
	{
		std::vector<olc::vi2d> outValidSquares;

		auto addSquareInX = [&](int x)
		{
			outValidSquares.push_back(screenToSquare(position, board) + olc::vi2d{ x, -1 });
			outValidSquares.push_back(screenToSquare(position, board) + olc::vi2d{ x, 1 });
		};

		auto addSquareInY = [&](int y)
		{
			outValidSquares.push_back(screenToSquare(position, board) + olc::vi2d{ -1, y });
			outValidSquares.push_back(screenToSquare(position, board) + olc::vi2d{ 1, y });
		};

		addSquareInX(2); addSquareInX(-2);
		addSquareInY(2); addSquareInY(-2);
		return outValidSquares;
	}
};

void DrawBoard(olc::PixelGameEngine* pge, const Board& board)
{
	bool color = true;
	for (int x = 0; x < board.nSquares.x; x++)
	{
		for (int y = 0; y < board.nSquares.y; y++)
		{
			pge->FillRectDecal(
				{ x * board.squareSize.x, y * board.squareSize.x },
				{ board.squareSize.y, board.squareSize.y },
				color ? olc::Pixel{ 170, 170, 170 } : olc::Pixel{ 80, 80, 80 });
			color = !color;
		}
		color = !color;
	}
}

void RenderPiece(olc::PixelGameEngine* pge, const Board& board, const Piece& piece, const olc::Pixel& color, bool moving)
{
	static olc::vf2d textOffset = { 10.0f, 25.0f };
	auto RenderInSquare = [&](const olc::vf2d& position, const std::string& text)
	{
		olc::vi2d square = screenToSquare(position, board);
		if (!moving)
		{
			pge->DrawStringDecal(square * board.squareSize + textOffset, text, color, { 1.0f, 3.0f });
		}
		else
		{
			pge->DrawStringDecal(piece.position, text, color, { 1.0f, 3.0f });
		}
	};

	Piece::Type type = piece.GetType();

	switch (type)
	{
	case Piece::Type::PAWN:
		RenderInSquare(piece.position, PieceString::pawn);
		break;
	case Piece::Type::BISHOP:
		RenderInSquare(piece.position, PieceString::bishop);
		break;
	case Piece::Type::KING:
		RenderInSquare(piece.position, PieceString::king);
		break;
	case Piece::Type::KNIGHT:
		RenderInSquare(piece.position, PieceString::knight);
		break;
	case Piece::Type::QUEEN:
		RenderInSquare(piece.position, PieceString::queen);
		break;
	case Piece::Type::ROOK:
		RenderInSquare(piece.position, PieceString::rook);
		break;
	}
}

class Controller
{
public:
	Controller(const olc::PixelGameEngine* const pge) :
		pge{ pge }
	{
	}

public:
	void LetUserDragDropPieces(std::vector<Piece*> pieces, const Board& board)
	{
		UpdateGrabbedPieceMoving(pieces, board);
		if (moving && grabbedPiece != nullptr)
		{
			grabbedPiece->position = (olc::vf2d)pge->GetMousePos();
		}
	}

	Piece* GetGrabbedPiece() const
	{
		if (grabbedPiece != nullptr)
			return grabbedPiece;
		return nullptr;
	}

	olc::vf2d GetLastPosition()
	{
		return lastPosition;
	}

	Piece* GetLastGrabbedpiece() const
	{
		if (lastGrabbedPiece != nullptr)
			return lastGrabbedPiece;
		return nullptr;
	}

private:
	olc::vi2d GetMouseInSquare(const Board& board)
	{
		olc::vi2d mouse = pge->GetMousePos();
		return screenToSquare((olc::vf2d)mouse, board);
	}

	void UpdateGrabbedPieceMoving(std::vector<Piece*> pieces, const Board& board)
	{
		for (auto& piece : pieces)
		{
			if (pge->GetMouse(0).bPressed)
			{
				lastGrabbedPiece = getPieceInSquare(GetMouseInSquare(board), pieces, board);
				if (lastGrabbedPiece != nullptr)
				{
					lastPosition = lastGrabbedPiece->position;
				}
			}

			if (pge->GetMouse(0).bHeld)
			{
				if (grabbedPiece == nullptr)
				{
					grabbedPiece = getPieceInSquare(GetMouseInSquare(board), pieces, board);
					if (grabbedPiece != nullptr)
					{
						moving = true;
					}
				}
			}
		}

		if (pge->GetMouse(0).bReleased)
		{
			moving = false;
			grabbedPiece = nullptr;
		}
	}

private:
	const olc::PixelGameEngine* const pge;
	Piece* grabbedPiece = nullptr;
	Piece* lastGrabbedPiece = nullptr;
	olc::vf2d lastPosition{};
	bool moving = true;
};

void DrawOccupiableSquares(olc::PixelGameEngine* pge, const std::vector<Piece*>& pieces, const olc::vf2d& lastPosition, const Board& board, const Piece& piece)
{

	std::vector<olc::vi2d> squares = piece.GetValidSquares(pieces, lastPosition, board);

	for (auto& square : squares)
	{
		pge->FillRectDecal(squareToScreen(square, board), board.squareSize, olc::Pixel{ 100, 250, 100, 80 });
	}
	
}

void ReturnToLastPosition(olc::PixelGameEngine* pge, const std::vector<Piece*>& pieces, const olc::vf2d& lastPosition, Piece& piece, const Board& board)
{
	if (pge->GetMouse(0).bReleased)
	{
		bool notReturn = false;
		std::vector<olc::vi2d> squares = piece.GetValidSquares(pieces, lastPosition, board);
		for (auto& square : squares)
		{
			notReturn |= screenToSquare(piece.position, board) == square;
		}

		if (squares.empty() || !notReturn)
		{
			piece.position = lastPosition;
		}
	}
}

class ChessGame : public olc::PixelGameEngine
{
public:
	ChessGame()
	{
		sAppName = "Chess Game";
	}

	~ChessGame()
	{
		for (int i = 0; i < nTotalPieces; i++)
		{
			delete pieces[i];
		}
	}

private:
	void InitPieces()
	{
		pieces.clear();
		for (int i = 0; i < 8; i++)
		{
			pieces.push_back(new Pawn{ squareToScreen({i, 1}, board), Piece::Color::BLACK });
			pieces.push_back(new Pawn{ squareToScreen({i, 6}, board), Piece::Color::WHITE });
		}

		pieces.push_back(new King{ squareToScreen({4, 0}, board), Piece::Color::BLACK });
		pieces.push_back(new King{ squareToScreen({4, 7}, board), Piece::Color::WHITE });
		
		pieces.push_back(new Queen{ squareToScreen({3, 0}, board), Piece::Color::BLACK });
		pieces.push_back(new Queen{ squareToScreen({3, 7}, board), Piece::Color::WHITE });
		
		pieces.push_back(new Rook{ squareToScreen({0, 0}, board), Piece::Color::BLACK });
		pieces.push_back(new Rook{ squareToScreen({0, 7}, board), Piece::Color::WHITE });
		
		pieces.push_back(new Rook{ squareToScreen({7, 0}, board), Piece::Color::BLACK });
		pieces.push_back(new Rook{ squareToScreen({7, 7}, board), Piece::Color::WHITE });
		
		pieces.push_back(new Bishop{ squareToScreen({2, 0}, board), Piece::Color::BLACK });
		pieces.push_back(new Bishop{ squareToScreen({2, 7}, board), Piece::Color::WHITE });
		
		pieces.push_back(new Bishop{ squareToScreen({5, 0}, board), Piece::Color::BLACK });
		pieces.push_back(new Bishop{ squareToScreen({5, 7}, board), Piece::Color::WHITE });
		
		pieces.push_back(new Knight{ squareToScreen({6, 0}, board), Piece::Color::BLACK });
		pieces.push_back(new Knight{ squareToScreen({6, 7}, board), Piece::Color::WHITE });
		
		pieces.push_back(new Knight{ squareToScreen({1, 0}, board), Piece::Color::BLACK });
		pieces.push_back(new Knight{ squareToScreen({1, 7}, board), Piece::Color::WHITE });

	}

public:
	bool OnUserCreate() override
	{
		board = Board({ ScreenWidth(), ScreenHeight() }, { 8, 8 });
		InitPieces();

		decalLayer = CreateLayer();
		//EnableLayer(decalLayer, true);

		return true;
	}

	bool OnUserUpdate(float elapsedTime) override
	{
		controller.LetUserDragDropPieces(pieces, board);
		Piece* lastGrabbed = controller.GetLastGrabbedpiece();
		if (lastGrabbed != nullptr)
		{
			ReturnToLastPosition(this, pieces, controller.GetLastPosition(), *lastGrabbed, board);
		}

		//Drawing
		/*Clear(olc::BLANK);
		SetDrawTarget(decalLayer);
		DrawBoard(this, board);

		for (int i = 0; i < nTotalPieces; i++)
		{
			Piece& piece = *pieces[i];
			RenderPiece(this, board, piece, piece.GetColor() == Piece::Color::BLACK ? olc::BLACK : olc::WHITE, piece == *controller.GetGrabbedPiece());
		}

		Piece* currentGrabbed = controller.GetGrabbedPiece();
		if (currentGrabbed != nullptr)
		{
			SetDrawTarget(nullptr);
			SetPixelMode(olc::Pixel::ALPHA);
			DrawOccupiableSquares(this, pieces, controller.GetLastPosition(), board, *currentGrabbed);
			SetPixelMode(olc::Pixel::NORMAL);
		}*/


		DrawBoard(this, board);
		Piece* currentGrabbed = controller.GetGrabbedPiece();
		if (currentGrabbed != nullptr)
		{
			DrawOccupiableSquares(this, pieces, controller.GetLastPosition(), board, *currentGrabbed);
		}

		for (int i = 0; i < nTotalPieces; i++)
		{
			Piece& piece = *pieces[i];
			RenderPiece(this, board, piece, piece.GetColor() == Piece::Color::BLACK ? olc::BLACK : olc::WHITE, piece == *controller.GetGrabbedPiece());
		}
		return true;
	}

private:
	static constexpr int nTotalPieces = 16 * 2;
	std::vector<Piece*> pieces{ };
	int decalLayer;
	Board board;
	Controller controller{ this };
};

int main()
{
	ChessGame game;
	if (game.Construct(600, 600, 1, 1))
		game.Start();
	return 0;
}

Piece::Piece(const olc::vf2d& position, Color color, MovementValidator* validator) :
	position{ position }, color{ color }, movementValidator{ validator }
{
}

Piece::~Piece() { delete movementValidator; };

Pawn::Pawn(const olc::vf2d& position, Color color) :
	Piece{ position, color, new PawnMovementValidator{} }
{
}

King::King(const olc::vf2d& position, Color color) :
	Piece{ position, color, new KingMovementValidator{} }
{
}

Queen::Queen(const olc::vf2d& position, Color color) :
	Piece{ position, color, new QueenMovementValidator{} }
{
}

Rook::Rook(const olc::vf2d& position, Color color) :
	Piece{ position, color, new RookMovementValidator{} }
{
}

Bishop::Bishop(const olc::vf2d& position, Color color) :
	Piece{ position, color, new BishopMovementValidator{} }
{
}

Knight::Knight(const olc::vf2d& position, Color color) :
	Piece{ position, color, new KnightMovementValidator{} }
{
}

std::vector<olc::vi2d> Piece::GetValidSquares(std::vector<Piece*> pieces, const olc::vf2d& position, const Board& board) const
{
	return movementValidator->GetOccupiableSquares(pieces, position, *this, board);
}
