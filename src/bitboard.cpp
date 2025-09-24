#include "bitboard.h"
#include <algorithm>
#include <bitset>
#include <cassert>

std::array<std::array<std::array<uint64_t, 2>, 8>, 8> BitBoard::zobristTable;
uint64_t BitBoard::zobristBlackToMoveKey = 0;
std::once_flag BitBoard::zobristOnce;
bool BitBoard::zobristInitialised = false;

BitBoard::BitBoard() { reset(); }

BitBoard::BitBoard(uint64_t black, uint64_t white)
    : blackBoard(black), whiteBoard(white) {
  initializeZobrist();
  hash_ = 0;
  uint64_t tmp = blackBoard;
  while (tmp) {
    int bit = __builtin_ctzll(tmp);
    tmp &= tmp - 1;
    auto [r, c] = bitToPosition(bit);
    hash_ ^= zobristTable[r][c][0];
  }
  tmp = whiteBoard;
  while (tmp) {
    int bit = __builtin_ctzll(tmp);
    tmp &= tmp - 1;
    auto [r, c] = bitToPosition(bit);
    hash_ ^= zobristTable[r][c][1];
  }
}

void BitBoard::reset() {
  initializeZobrist();
  blackBoard = 0;
  whiteBoard = 0;
  hash_ = 0;
  setCell(3, 4, 1);
  setCell(4, 3, 1);
  setCell(3, 3, 2);
  setCell(4, 4, 2);
}

int BitBoard::positionToBit(int row, int col) { return row * BOARD_SIZE + col; }

std::pair<int, int> BitBoard::bitToPosition(int bit) {
  return {bit / BOARD_SIZE, bit % BOARD_SIZE};
}

uint64_t BitBoard::positionToMask(int row, int col) {
  return 1ULL << positionToBit(row, col);
}

int BitBoard::getCell(int row, int col) const {
  if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
    throw std::out_of_range("Cell coordinates out of bounds");
  }
  uint64_t mask = positionToMask(row, col);
  if (blackBoard & mask)
    return 1;
  if (whiteBoard & mask)
    return 2;
  return 0;
}

void BitBoard::setCell(int row, int col, int state) {
  if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
    throw std::out_of_range("Cell coordinates out of bounds");
  }
  uint64_t mask = positionToMask(row, col);
  int prevState = 0;
  if (blackBoard & mask)
    prevState = 1;
  else if (whiteBoard & mask)
    prevState = 2;
  if (prevState == 1) {
    hash_ ^= zobristTable[row][col][0];
  } else if (prevState == 2) {
    hash_ ^= zobristTable[row][col][1];
  }
  blackBoard &= ~mask;
  whiteBoard &= ~mask;
  if (state == 1) {
    blackBoard |= mask;
    hash_ ^= zobristTable[row][col][0];
  } else if (state == 2) {
    whiteBoard |= mask;
    hash_ ^= zobristTable[row][col][1];
  }
}

uint64_t BitBoard::getPlayerBoard(bool isBlack) const {
  return isBlack ? blackBoard : whiteBoard;
}

uint64_t BitBoard::getCornerMask() const {
  static const uint64_t cornerMask = []() {
    uint64_t m = 0;
    m |= positionToMask(0, 0);
    m |= positionToMask(0, 7);
    m |= positionToMask(7, 0);
    m |= positionToMask(7, 7);
    return m;
  }();
  return cornerMask;
}

uint64_t BitBoard::getEdgeMask() const {
  static const uint64_t edgeMask = []() {
    uint64_t m = 0;
    for (int c = 0; c < BOARD_SIZE; ++c) {
      m |= positionToMask(0, c);
      m |= positionToMask(BOARD_SIZE - 1, c);
    }
    for (int r = 1; r < BOARD_SIZE - 1; ++r) {
      m |= positionToMask(r, 0);
      m |= positionToMask(r, BOARD_SIZE - 1);
    }
    uint64_t corners = positionToMask(0, 0) | positionToMask(0, 7) |
                       positionToMask(BOARD_SIZE - 1, 0) |
                       positionToMask(BOARD_SIZE - 1, BOARD_SIZE - 1);
    return m & ~corners;
  }();
  return edgeMask;
}

uint64_t BitBoard::getStableMask() const { return 0ULL; }

uint64_t BitBoard::getOpponentBoard(bool isBlack) const {
  return isBlack ? whiteBoard : blackBoard;
}

bool BitBoard::isValidMove(int row, int col, bool isBlack) const {
  if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) {
    return false;
  }
  if (getCell(row, col) != 0)
    return false;
  return getFlippedBitboard(row, col, isBlack) != 0;
}

std::vector<std::pair<int, int>> BitBoard::getValidMoves(bool isBlack) const {
  std::vector<std::pair<int, int>> moves;
  for (int r = 0; r < BOARD_SIZE; ++r) {
    for (int c = 0; c < BOARD_SIZE; ++c) {
      if (isValidMove(r, c, isBlack)) {
        moves.emplace_back(r, c);
      }
    }
  }
  return moves;
}

uint64_t BitBoard::getFlippedBitboard(int row, int col, bool isBlack) const {
  uint64_t flipped = 0;
  uint64_t playerBoard = getPlayerBoard(isBlack);
  uint64_t opponentBoard = getOpponentBoard(isBlack);
  static const std::array<std::pair<int, int>, 8> directions = {
      std::make_pair(-1, 0), // N
      std::make_pair(-1, 1), // NE
      std::make_pair(0, 1),  // E
      std::make_pair(1, 1),  // SE
      std::make_pair(1, 0),  // S
      std::make_pair(1, -1), // SW
      std::make_pair(0, -1), // W
      std::make_pair(-1, -1) // NW
  };
  for (auto [dr, dc] : directions) {
    int r = row + dr;
    int c = col + dc;
    uint64_t temp = 0;
    bool foundOpponent = false;
    while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE) {
      uint64_t mask = positionToMask(r, c);
      if (opponentBoard & mask) {
        temp |= mask;
        foundOpponent = true;
      } else if (playerBoard & mask) {
        if (foundOpponent) {
          flipped |= temp;
        }
        break;
      } else {
        break;
      }
      r += dr;
      c += dc;
    }
  }
  return flipped;
}

std::vector<std::pair<int, int>> BitBoard::getFlippedDiscs(int row, int col,
                                                           bool isBlack) const {
  std::vector<std::pair<int, int>> discs;
  uint64_t flipMask = getFlippedBitboard(row, col, isBlack);
  while (flipMask) {
    int bit = __builtin_ctzll(flipMask);
    auto [r, c] = bitToPosition(bit);
    discs.emplace_back(r, c);
    flipMask &= flipMask - 1;
  }
  return discs;
}

bool BitBoard::makeMove(int row, int col, bool isBlack) {
  if (!isValidMove(row, col, isBlack)) {
    return false;
  }
  uint64_t moveMask = positionToMask(row, col);
  uint64_t flipped = getFlippedBitboard(row, col, isBlack);
  auto [pr, pc] = bitToPosition(__builtin_ctzll(moveMask));
  if (isBlack) {
    hash_ ^= zobristTable[pr][pc][0];
  } else {
    hash_ ^= zobristTable[pr][pc][1];
  }
  uint64_t tmp = flipped;
  while (tmp) {
    int bit = __builtin_ctzll(tmp);
    tmp &= tmp - 1;
    auto [r, c] = bitToPosition(bit);
    if (isBlack) {
      hash_ ^= zobristTable[r][c][1];
      hash_ ^= zobristTable[r][c][0];
    } else {
      hash_ ^= zobristTable[r][c][0];
      hash_ ^= zobristTable[r][c][1];
    }
  }
  if (isBlack) {
    blackBoard |= moveMask;
    blackBoard |= flipped;
    whiteBoard &= ~flipped;
  } else {
    whiteBoard |= moveMask;
    whiteBoard |= flipped;
    blackBoard &= ~flipped;
  }
  return true;
}

bool BitBoard::hasValidMoves(bool isBlack) const {
  if (isFull())
    return false;
  for (int r = 0; r < BOARD_SIZE; ++r) {
    for (int c = 0; c < BOARD_SIZE; ++c) {
      if (isValidMove(r, c, isBlack)) {
        return true;
      }
    }
  }
  return false;
}

bool BitBoard::isGameOver() const {
  return !hasValidMoves(true) && !hasValidMoves(false);
}

bool BitBoard::isFull() const {
  return __builtin_popcountll(blackBoard | whiteBoard) == TOTAL_CELLS;
}

int BitBoard::getScore(bool isBlack) const {
  return __builtin_popcountll(isBlack ? blackBoard : whiteBoard);
}

int BitBoard::getTotalDiscs() const {
  return __builtin_popcountll(blackBoard | whiteBoard);
}

bool BitBoard::operator==(const BitBoard &other) const {
  return blackBoard == other.blackBoard && whiteBoard == other.whiteBoard;
}

bool BitBoard::operator!=(const BitBoard &other) const {
  return !(*this == other);
}

uint64_t BitBoard::shiftMask(uint64_t board, int dr, int dc) {
  uint64_t result = 0;
  while (board) {
    int bit = __builtin_ctzll(board);
    board &= board - 1;
    auto [r, c] = bitToPosition(bit);
    int nr = r + dr;
    int nc = c + dc;
    if (nr >= 0 && nr < BOARD_SIZE && nc >= 0 && nc < BOARD_SIZE) {
      result |= positionToMask(nr, nc);
    }
  }
  return result;
}

static inline uint64_t splitmix64(uint64_t &x) {
  uint64_t z = (x += 0x9e3779b97f4a7c15ULL);
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
  return z ^ (z >> 31);
}

void BitBoard::initializeZobrist(uint64_t seed) {
  static bool once = [&]() {
    uint64_t s = (seed == 0) ? 0x0123456789ABCDEFULL : seed;
    for (int r = 0; r < 8; ++r) {
      for (int c = 0; c < 8; ++c) {
        for (int p = 0; p < 2; ++p) {
          zobristTable[r][c][p] = splitmix64(s);
        }
      }
    }
    zobristBlackToMoveKey = splitmix64(s);
    zobristInitialised = true;
    return true;
  }();
  (void)once;
}

uint64_t BitBoard::getZobristKey(int row, int col, int player01) {
  if (!zobristInitialised) {
    initializeZobrist();
  }
  assert(player01 == 0 || player01 == 1);
  return zobristTable[row][col][player01];
}

uint64_t BitBoard::getZobristHash() const {
  if (!zobristInitialised) {
    initializeZobrist();
  }
  return hash_;
}

uint64_t BitBoard::getZobristHash(bool blackToMove) const {
  const_cast<BitBoard *>(this)->initializeZobrist();
  uint64_t h = getZobristHash();
  if (blackToMove)
    h ^= zobristBlackToMoveKey;
  return h;
}

uint64_t BitBoard::getCanonicalZobristHash() const {
    BitBoard canonical = toCanonical();
    return canonical.getZobristHash();
}

BitBoard BitBoard::toCanonical() const {
    BitBoard candidates[8];
    candidates[0] = *this;
    candidates[1] = this->rot90();
    candidates[2] = this->rot180();
    candidates[3] = this->rot270();
    candidates[4] = this->mirrorHorizontal();
    candidates[5] = this->mirrorVertical();
    candidates[6] = this->mirrorDiag();
    candidates[7] = this->mirrorAntiDiag();

    BitBoard best = candidates[0];
    for (int i = 1; i < 8; ++i) {
        if (candidates[i] < best) {
            best = candidates[i];
        }
    }
    return best;
}

BitBoard BitBoard::rot90() const {
    return {
        flipDiagonal(flipHorizontal(blackBoard)),
        flipDiagonal(flipHorizontal(whiteBoard))
    };
}

BitBoard BitBoard::rot180() const {
    return {
        flipVertical(flipHorizontal(blackBoard)),
        flipVertical(flipHorizontal(whiteBoard))
    };
}

BitBoard BitBoard::rot270() const {
    return {
        flipAntiDiagonal(flipHorizontal(blackBoard)),
        flipAntiDiagonal(flipHorizontal(whiteBoard))
    };
}

BitBoard BitBoard::mirrorHorizontal() const {
    return {
        flipHorizontal(blackBoard),
        flipHorizontal(whiteBoard)
    };
}

BitBoard BitBoard::mirrorVertical() const {
    return {
        flipVertical(blackBoard),
        flipVertical(whiteBoard)
    };
}

BitBoard BitBoard::mirrorDiag() const {
    return {
        flipDiagonal(blackBoard),
        flipDiagonal(whiteBoard)
    };
}

BitBoard BitBoard::mirrorAntiDiag() const {
    return {
        flipAntiDiagonal(blackBoard),
        flipAntiDiagonal(whiteBoard)
    };
}

uint64_t BitBoard::getStableMaskForPlayerBitwise(bool isBlack) const {
    const uint64_t player = getPlayerBoard(isBlack);

    // Masks to prevent wraparound during bit shifts
    const uint64_t NOT_LEFT   = 0xfefefefefefefefeULL;
    const uint64_t NOT_RIGHT  = 0x7f7f7f7f7f7f7f7fULL;
    const uint64_t NOT_TOP    = 0xffffffffffffff00ULL;
    const uint64_t NOT_BOTTOM = 0x00ffffffffffffffULL;

    // Initial stable positions (corners)
    uint64_t stable = 0;
    if (player & positionToMask(0, 0)) stable |= positionToMask(0, 0);
    if (player & positionToMask(0, 7)) stable |= positionToMask(0, 7);
    if (player & positionToMask(7, 0)) stable |= positionToMask(7, 0);
    if (player & positionToMask(7, 7)) stable |= positionToMask(7, 7);

    uint64_t prev = 0;

    while (stable != prev) {
        prev = stable;

        // All player stones not yet marked as stable
        uint64_t candidates = player & ~stable;

        // Build masks of stable directions
        uint64_t north = ((stable << 8) & NOT_TOP);
        uint64_t south = ((stable >> 8) & NOT_BOTTOM);
        uint64_t east  = ((stable >> 1) & NOT_RIGHT);
        uint64_t west  = ((stable << 1) & NOT_LEFT);
        uint64_t ne = ((stable << 7) & NOT_TOP & NOT_RIGHT);
        uint64_t nw = ((stable << 9) & NOT_TOP & NOT_LEFT);
        uint64_t se = ((stable >> 9) & NOT_BOTTOM & NOT_RIGHT);
        uint64_t sw = ((stable >> 7) & NOT_BOTTOM & NOT_LEFT);

        // Keep only candidates that are fully surrounded in all directions
        uint64_t surrounded =
            north & south & east & west & ne & nw & se & sw;

        // Filter to only player's pieces
        surrounded &= candidates;

        // Add to stable set
        stable |= surrounded;
    }

    return stable;
}

bool BitBoard::isCornerControlled(int r, int c, bool isBlack) const {
    int corner_r = (r < 4) ? 0 : 7;
    int corner_c = (c < 4) ? 0 : 7;
    int owner = getCell(corner_r, corner_c);
    return (isBlack && owner == 1) || (!isBlack && owner == 2);
}

static constexpr uint64_t cornerMask = 
    (1ULL << 0) |          // (0,0)
    (1ULL << 7) |          // (0,7)
    (1ULL << 56) |         // (7,0)
    (1ULL << 63);          // (7,7)

uint64_t BitBoard::getPlayerCorners(bool isBlack) const {
    uint64_t playerBoard = isBlack ? blackBoard : whiteBoard;
    return playerBoard & cornerMask;  // bits à 1 où le joueur a un coin
}
