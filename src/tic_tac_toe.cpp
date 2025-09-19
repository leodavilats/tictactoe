#include <iostream>
#include <random>
#include <thread>
#include <array>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <ctime>
#include <exception>

// Classe TicTacToe
class TicTacToe {
  public:
  mutable std::mutex board_mutex;
  std::condition_variable turn_cv;
  
  private:
  std::array<std::array<char, 3>, 3> board;
  char current_player;
  bool game_over;
  char winner;
  
  public:
  TicTacToe() {
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        board[i][j] = ' ';
      }
    }
    winner = '-';
    game_over = false;
    static std::mt19937 sorteiaJogador(static_cast<unsigned int>(std::time(0)));
    static std::uniform_int_distribution<int> distr(0, 1);
    current_player = distr(sorteiaJogador) == 0 ? 'X' : 'O';
    
    std::cout << "Jogo iniciado! Jogador inicial: " << current_player << std::endl;
  }
  
  void display_board() {
    std::lock_guard<std::mutex> lock(board_mutex);
    
    #ifdef _WIN32
        std::system("cls");
    #else
        std::system("clear");
    #endif
    
    std::cout << "\nTic Tac Toe - Jogador atual: " << current_player << "\n\n";
    
    for(int i = 0; i < 3; i++){
      std::cout << " " << board[i][0] << " | " << board[i][1] << " | " << board[i][2] << std::endl;
      if(i != 2){
        std::cout << "---|---|---" << std::endl;
      }
    }
    std::cout << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
  }
  
  bool make_move(char player, int row, int col) {
    std::unique_lock<std::mutex> lock(board_mutex);
    
    if (current_player != player || game_over) {
      return false;
    }
    
    if (board[row][col] != ' ') {
      return false;
    }
    
    board[row][col] = player;
    
    if (check_win(player)) {
      game_over = true;
      winner = player;
    } else if (check_draw()) {
      game_over = true;
      winner = 'D';
    } else {
      current_player = (current_player == 'X') ? 'O' : 'X';
    }
    
    lock.unlock();
    
    display_board();
    
    turn_cv.notify_all();
    
    return true;
  }
  
  bool check_win(char player) {
    for(int i = 0; i < 3; i++){
      if(player == board[i][0] && player == board[i][1] && player == board[i][2]){
        winner = player;
        return 1;
      }
    }
    for(int i = 0; i < 3; i++){
      if(player == board[0][i] && player == board[1][i] && player == board[2][i]){
        winner = player;
        return 1;
      }
    }
    if(player == board[0][0] && player == board[1][1] && player == board[2][2]){
      winner = player;
      return 1;
    }
    if(player == board[0][2] && player == board[1][1] && player == board[2][0]){
      winner = player;
      return 1;
    }
    return 0;
  }
  
  bool check_draw() {
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        if(board[i][j] == ' '){
          return 0;
        }
      }
    }
    return 1;
  }
  
  bool is_game_over() {
    if(check_win(current_player)){
      return 1;
    }else if(check_draw()){
      winner = 'D';
      return 1;
    }else{
      winner = '-';
      return 0;
    }
  }
  
  char get_winner() {
    return winner;
  }
  
  char get_current_player() {
    std::lock_guard<std::mutex> lock(board_mutex);
    return current_player;
  }
  
  bool is_player_turn(char player) {
    std::lock_guard<std::mutex> lock(board_mutex);
    return current_player == player && !game_over;
  }
  
  bool is_game_finished() {
    std::lock_guard<std::mutex> lock(board_mutex);
    return game_over;
  }
};

// Classe Player
class Player {
  private:
  TicTacToe& game;
  char symbol;
  std::string strategy;
  
  public:
  Player(TicTacToe& g, char s, std::string strat) 
  : game(g), symbol(s), strategy(strat) {}
  
  void play() {
    while(game.get_winner() == '-' && !game.is_game_finished()){
      if (!game.is_player_turn(symbol)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        continue;
      }
      
      bool move_made = false;
      if(strategy == "sequential"){
        move_made = play_sequential();
      }else{
        move_made = play_random();
      }
      
      if (!move_made) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    }
  }
  
  private:
  bool play_sequential() {
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        if(game.make_move(symbol, i, j)){
          return true;
        }
      }
    }
    return false;
  }
  
  bool play_random() {
    int l;
    int c;
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> distr(0, 2);
    
    for (int attempts = 0; attempts < 10; ++attempts) {
      l = distr(gen);
      c = distr(gen);
      if (game.make_move(symbol, l, c)) {
        return true;
      }
    }
    return false;
  }
};

int main() {
  try {
    std::cout << "Iniciando Jogo da Velha Concorrente..." << std::endl;
    TicTacToe tabuleiro;
    tabuleiro.display_board();
    
    Player X(tabuleiro, 'X', "sequential");
    Player O(tabuleiro, 'O', "random");
    
    std::cout << "Criando threads dos jogadores..." << std::endl;
    
    std::thread Jogador1(&Player::play, &X);
    std::thread Jogador2(&Player::play, &O);
    
    Jogador1.join();
    Jogador2.join();
    
    char vencedor = tabuleiro.get_winner();
    std::cout << "\n=== RESULTADO FINAL ===" << std::endl;
    if(vencedor == 'D'){
      std::cout << "Empate!" << std::endl;
    } else {
      std::cout << "Vencedor: " << vencedor << std::endl;
    }
    
  } catch (const std::exception& e) {
    std::cerr << "Erro: " << e.what() << std::endl;
    return 1;
  }
  
  return 0;
}
