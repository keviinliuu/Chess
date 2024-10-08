#include "Game.h"
#include "../interpreter/Commands.h"
#include "player/Human.h"
#include "player/computer/Noob.h"
#include "player/computer/Intermediate.h"
#include "player/computer/Pro.h"
#include "player/computer/Grandmaster.h"
#include <iostream>
using namespace std;

Game::Game(): board{std::make_shared<Board>()}, interpreter{std::make_unique<Interpreter>()}{
    board->initializeStandardBoard();
}

Game::~Game() {}

bool Game::checkCheck(Colour colour, const shared_ptr<Board>& board) {
    // Colour opponentColour = (colour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
    std::shared_ptr<Tile> kingTile = (colour == Colour::WHITE) ? board->getWhiteKingTile() : board->getBlackKingTile();
    auto king = std::dynamic_pointer_cast<King>(kingTile->getPiece());
    return king->isInCheck(*board, kingTile->getRow(), kingTile->getCol());
}

bool Game::canMakeLegalMove(Colour colour) {
    for (int row = 0; row < board->getBoardSize(); ++row) {
        for (int col = 0; col < board->getBoardSize(); ++col) {
            std::shared_ptr<Piece> piece = board->getTile(row, col)->getPiece();
            if (piece && piece->getColour() == colour) {
                for (int endRow = 0; endRow < board->getBoardSize(); ++endRow) {
                    for (int endCol = 0; endCol < board->getBoardSize(); ++endCol) {
                        if (piece->isValidMove(*board, row, col, endRow, endCol)) {
                            std::shared_ptr<Board> tempBoard = std::make_shared<Board>(*board);
                            tempBoard->movePiece(row, col, endRow, endCol);
                            std::shared_ptr<Tile> tempKingTile = (colour == Colour::WHITE) ? tempBoard->getWhiteKingTile() : tempBoard->getBlackKingTile();
                            auto tempKing = std::dynamic_pointer_cast<King>(tempKingTile->getPiece());
                            // Check if the king is still not in check after the move
                            if (!tempKing->isInCheck(*tempBoard, tempKingTile->getRow(), tempKingTile->getCol())) {
                                return true; // Found a legal move
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool Game::checkCheckmate(Colour colour) {
    std::shared_ptr<Tile> kingTile = (colour == Colour::WHITE) ? board->getWhiteKingTile() : board->getBlackKingTile();
    auto king = std::dynamic_pointer_cast<King>(kingTile->getPiece());

    if (!king->isInCheck(*board, kingTile->getRow(), kingTile->getCol())) {
        return false;
    }

    if (canMakeLegalMove(colour)) {
        return false;
    }

    return true; // King is in checkmate
}


bool Game::checkStalemate(Colour colour) {
    std::shared_ptr<Tile> kingTile = (colour == Colour::WHITE) ? board->getWhiteKingTile() : board->getBlackKingTile();
    auto king = std::dynamic_pointer_cast<King>(kingTile->getPiece());
    
    if (king->isInCheck(*board, kingTile->getRow(), kingTile->getCol())) {
        return false;
    }

    if (canMakeLegalMove(colour)) {
        return false;
    }

    return true; // No valid moves found
}

bool Game::checkPromotion(const shared_ptr<Board>& board, Move move) {
    if (auto pawn = dynamic_pointer_cast<Pawn>(board->getTile(move.endRow, move.endCol)->getPiece())) {
        if (move.endRow == 0 || move.endRow == 7) {
            return true;
        }
    }
    return false;
}

void Game::gameLoop(){ 
    std::cout << "New game has started!" << endl;
    board->setGameStatus(NOSTATUS);
    bool gameOver = false;

    //our loop in game mode

    while(true){
        //PLAYER1 input loop
        while(nextPlayer == 1){
            board->render();
            std::cout << "WHITE'S TURN" << endl << endl;
            std::cout << "Enter a game command:" << endl;
            //call makeMove for player1 (returns a Move class)
            Move player1Move = player1->makeMove(interpreter, board);

            if(player1Move.isResign){
                board->setGameStatus(BLACKWINS);
                p2score += 1;
                gameOver = true;
                break;
            }
            
            if(player1Move.startCol < 0){ //invalid move
                if(player1Move.promotionType == 'q'){ //quit move
                    return;
                }else{
                    std::cout << "Invalid command in this context. Please try again." << endl;
                    continue; //retry

                }
            }

            //check if the move is a valid move
            if(!board->getTile(player1Move.startRow, player1Move.startCol)->getPiece() ||
                board->getTile(player1Move.startRow, player1Move.startCol)->getPiece()->getColour() != WHITE ||
                !board->getTile(player1Move.startRow, player1Move.startCol)->getPiece()
                ->isValidMove(*board, player1Move.startRow, player1Move.startCol, player1Move.endRow, player1Move.endCol)){
            
                //not valid move
                std::cout << "Invalid piece move. Please try again." << endl;
                continue;
            }

            //make tempboard
            //check if player1 in check, if yes reject
            shared_ptr<Board> tempboard = make_shared<Board>(*board);
            tempboard->movePiece(player1Move.startRow, player1Move.startCol, player1Move.endRow, player1Move.endCol);
            
            if(checkCheck(WHITE, tempboard)){
                std::cout << "White put itself in check. Invalid move." << endl;
                continue;
            }

            //check if we have everything for a promotion if relevant
            if(checkPromotion(tempboard, player1Move) && player1Move.promotionType == 0){
                std::cout << "Need a promotion type." << endl;
                continue;
            }
            
            //make move on real board
            board->movePiece(player1Move.startRow, player1Move.startCol, player1Move.endRow, player1Move.endCol);

            //update piece (if promotion)
            if(checkPromotion(board, player1Move)){
                board->addPiece(player1Move.endRow, player1Move.endCol, tolower(player1Move.promotionType), WHITE);
            }

            nextPlayer = 2;

            //check checkCheckmate, checkStalemate, checkCheck for player2
            if(checkCheckmate(BLACK)){
                board->setGameStatus(CHECKMATEWHITE);
                p1score += 1;
                gameOver = true;
            }
            else if(checkCheck(BLACK, board)){
                board->setGameStatus(BLACKCHECK);
            }
            else if(checkStalemate(BLACK)){
                board->setGameStatus(STALEMATE);
                p1score += 0.5;
                p2score += 0.5;
                gameOver = true;
            }
            else {
                board->setGameStatus(NOSTATUS);
            }
        }

        if(gameOver) {
            board->render();
            break;
        }
        
        //PLAYER2 input loop

        while(nextPlayer == 2){
            board->render();
            std::cout << "BLACK'S TURN" << endl << endl;
            std::cout << "Enter a game command:" << endl;
            //call makeMove for player2 (returns a Move class)
            Move player2Move = player2->makeMove(interpreter, board);

            if(player2Move.isResign){
                board->setGameStatus(WHITEWINS);
                p1score += 1;
                gameOver = true;
                break;
            }

            if(player2Move.startCol < 0){ //invalid move
                if(player2Move.promotionType == 'q'){ //quit move
                    return;
                }else{
                    std::cout << "Invalid command in this context. Please try again." << endl;
                    continue; //retry
                }
            }

            //check if the move is a valid move
            if(!board->getTile(player2Move.startRow, player2Move.startCol)->getPiece() ||
                board->getTile(player2Move.startRow, player2Move.startCol)->getPiece()->getColour() != BLACK ||
                !board->getTile(player2Move.startRow, player2Move.startCol)->getPiece()
                ->isValidMove(*board, player2Move.startRow, player2Move.startCol, player2Move.endRow, player2Move.endCol)){
            
                //not valid move
                std::cout << "Invalid piece move. Please try again." << endl;
                continue;
            }

            //make tempboard
            //check if player2 in check, if yes reject
            shared_ptr<Board> tempboard = make_shared<Board>(*board);
            tempboard->movePiece(player2Move.startRow, player2Move.startCol, player2Move.endRow, player2Move.endCol);

            if(checkCheck(BLACK, tempboard)){
                std::cout << "Black put itself in check. Invalid move." << endl;
                continue;
            }

            //check if we have everything for a promotion if relevant
            if(checkPromotion(tempboard, player2Move) && player2Move.promotionType == 0){
                std::cout << "Need a promotion type." << endl;
                continue;
            }
            
            //make move on real board
            board->movePiece(player2Move.startRow, player2Move.startCol, player2Move.endRow, player2Move.endCol);

            //update piece (if promotion)
            if(checkPromotion(board, player2Move)){
                board->addPiece(player2Move.endRow, player2Move.endCol, tolower(player2Move.promotionType), BLACK);
            }

            nextPlayer = 1;

            // check checkCheckmate, checkStalemate, checkCheck for player1
            if(checkCheckmate(WHITE)){
                board->setGameStatus(CHECKMATEBLACK);
                p2score += 1;
                gameOver = true;
            }
            else if(checkCheck(WHITE, board)){
                board->setGameStatus(WHITECHECK);
            }
            else if(checkStalemate(WHITE)){
                board->setGameStatus(STALEMATE);
                p1score += 0.5;
                p2score += 0.5;
                gameOver = true;
            }
            else {
                board->setGameStatus(NOSTATUS);
            }
        }

        if(gameOver) {
            board->render();
            break;
        }
    }

    board->clearBoard();
    board->initializeStandardBoard();
    nextPlayer = 1;
    board->setGameStatus(NOSTATUS);
}

void Game::setupLoop() {
    //our loop in set up mode

    cout << "You have entered set-up mode!" << endl;

    //clear board
    board->clearBoard();
    board->render();

    while(true){
        //read in commands from interpreter

        cout << "Enter a set-up command:" << endl;

        shared_ptr<Command> command = interpreter->readCommand();

        if(!command){
            cout << "Invalid command in this context. Please try again." << endl;
            continue;
        }


        if(command->getType() == DONESETUP){
            if(board->processValidBoard()){

                if((checkStalemate(WHITE) && nextPlayer == 1) || (checkStalemate(BLACK) && nextPlayer == 2)){ //if the next player is in stalemate
                    cout << "The next turn will be a stalemate. Please change the board before trying again." << endl;
                }else{
                    cout << "Leaving setup mode." << endl;
                    break;
                }
            }else{
                cout << "You are leaving the board in an invalid condition. Please change the board before trying again." << endl;
                continue;
            }
        }

        switch(command->getType()){
            case ADDPIECE:{
                shared_ptr<AddPiece> apc = dynamic_pointer_cast<AddPiece>(command);
                vector<int> pos = apc->getPos();
                char piece = apc->getPiece();

                Colour pieceColour = ('A' <= piece && piece <= 'Z' ? WHITE : BLACK);
                board->addPiece(pos[0], pos[1], tolower(piece), pieceColour);
                cout << "Successfully added piece." << endl;
                board->render();
                break;
            }

            case REMOVEPIECE:{
                shared_ptr<RemovePiece> rpc = dynamic_pointer_cast<RemovePiece>(command);
                vector<int> pos = rpc->getPos();
                bool rem_succ = board->removePiece(pos[0], pos[1]);
                if(rem_succ){
                    cout << "Successfully removed piece." << endl;
                    board->render();
                }else{
                    cout << "Did not remove a piece." << endl;
                }
                break;
            }

            case SETCOLOUR: {
                shared_ptr<SetColour> scc = dynamic_pointer_cast<SetColour>(command);

                nextPlayer = (scc->getColour() == WHITE ? 1 : 2);

                cout << "The next player is now Player " << nextPlayer << endl;
                break;
            }

            case QUIT: {
                return;
            }

            default:
                cout << "Invalid command in this context. Please try again" << endl;
                break;
        }
    }
}

void Game::runProgram(){
    std::cout << "Welcome to Chess!" << endl;
    
    shared_ptr<Command> command;

    while(true){
        std::cout << "Start a game or enter set-up mode:" << endl;
        //read command
        command = interpreter->readCommand();

        if(!command){
            std::cout << "Invalid command in this context. Please try again." << endl;
            continue;
        }

        switch(command->getType()){
            case STARTGAME: {
                shared_ptr<StartGame> sg_command = dynamic_pointer_cast<StartGame>(command);

                switch(sg_command->getWhitePlayer()){
                    case HUMAN:
                        player1 = std::make_shared<Human>(WHITE);
                        break;
                    case COMPUTER1:
                        player1 = std::make_shared<Noob>(WHITE);
                        break;
                    case COMPUTER2:
                        player1 = std::make_shared<Intermediate>(WHITE);
                        break;
                    case COMPUTER3:
                        player1 = std::make_shared<Pro>(WHITE);
                        break;
                    case COMPUTER4:
                        player1 = std::make_shared<Grandmaster>(WHITE);
                        break;
                }
                
                switch(sg_command->getBlackPlayer()){
                    case HUMAN:
                        player2 = std::make_shared<Human>(BLACK);
                        break;
                    case COMPUTER1:
                        player2 = std::make_shared<Noob>(BLACK);
                        break;
                    case COMPUTER2:
                        player2 = std::make_shared<Intermediate>(BLACK);
                        break;
                    case COMPUTER3:
                        player2 = std::make_shared<Pro>(BLACK);
                        break;
                    case COMPUTER4:
                        player2 = std::make_shared<Grandmaster>(BLACK);
                        break;
                }

                gameLoop();
                break;
            }

            case SETUP: {
                setupLoop();
                break;
            }

            case QUIT: {
                cout << "Exiting program..." << endl;
                cout << "Final score:" << endl;
                cout << "White: " << p1score << endl;
                cout << "Black: " << p2score << endl;
                return;
            }

            default:{
                std::cout << "Invalid command in this context. Please try again." << endl;
            }
        }
    }
}
