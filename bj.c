#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define CLEAR "cls" // only for windows, so don't assume this in exam or something

#define DECK_SIZE 52
static const char RANKS[] = "23456789TJQKA";
static const char SUITS[] = "DHSC";

#define MAX_HAND 21
#define MAX_DEALER_CARDS 17
#define MAX_PLAYER_CARDS 21


#define STARTING_BALANCE 1000 // starting balance in $
#define MIN_BET          10   // minimum bet in $

typedef struct {
    char rank;
    char suit;
} Card;

typedef struct {
    Card cards[DECK_SIZE];
    int top;
} Deck;

typedef struct {
    Card cards[MAX_HAND];
    int count;
} Hand;


// deck
void init_deck(Deck *d) {
    int i = 0;
    int r;
    int s;

    for (r = 0; r < 13; r = r + 1) {
        for (s = 0; s < 4; s = s + 1) {
            (*d).cards[i].rank = RANKS[r];
            (*d).cards[i].suit = SUITS[s];
            i = i + 1;
        }
    }
    (*d).top = 0;
}

void shuffle_deck(Deck *d) {
    int i;
    int j;
    Card temp;

    for (i = DECK_SIZE - 1; i > 0; i = i - 1) {
        j = rand() % (i + 1);
        temp = (*d).cards[i];
        (*d).cards[i] = (*d).cards[j];
        (*d).cards[j] = temp;
    }
}

Card deal_card(Deck *d) {
    return (*d).cards[(*d).top++];
}


// hand
void add_card(Hand *hand, Card card) {
    (*hand).cards[(*hand).count] = card;
    (*hand).count = (*hand).count + 1;
}

int calculate_hand_count(Hand *hand) {
    int count = 0;
    int aces = 0;
    int i;

    for (i = 0; i < (*hand).count; i++) {
        char r = (*hand).cards[i].rank;
        if (r >= '2' && r <= '9') {
            count += r - '0';
        } else if (r == 'T' || r == 'J' || r == 'Q' || r == 'K') {
            count += 10;
        } else if (r == 'A') {
            count += 11;
            aces = aces + 1;
        }
    }

    while (aces > 0 && count > 21) {
        count -= 10;
        aces = aces - 1;
    }
    return count;
}


// aesthetic stuff for the ui
void typewrite(const char *str, int delay_us) {
    int i = 0;
    while (str[i] != '\0') {
        putchar(str[i]);
        fflush(stdout);
        usleep(delay_us);
        i = i + 1;
    }
}

void animate_shuffle(void) {
    static const char *frames[] = {"|", "/", "-", "\\"};
    int i;

    printf("  Shuffling deck  ");
    fflush(stdout);
    for (i = 0; i < 16; i = i + 1) {
        printf("%s", frames[i % 4]);
        fflush(stdout);
        usleep(80000);
        printf("\b");
    }
    printf("Done!\n\n");
}

void animate_deal(Hand *hand, Card card, int hide) {
    usleep(350000);
    add_card(hand, card);
    if (hide) {
        printf("  [ X ]\n");
    } else {
        printf("  [ %c ]\n", card.rank);
    }
    fflush(stdout);
}

void typewrite_result(const char *str) {
    printf("\n  ");
    usleep(400000);
    typewrite(str, 60000);
    printf("\n");
}


// display for the ui
void clear_screen(void) {
    system(CLEAR);
}

void print_main_menu(void) {
    clear_screen();
    printf("\n");
    typewrite("  ===== WELCOME TO DANIEL'S CASINO =====\n", 25000);
    printf("\n");
    typewrite("  Press ENTER to start...\n", 25000);
    getchar();
}

void print_hand(const char *label, Hand *hand, int hide_second) {
    int i;
    int pad;

    printf("  %-8s:  ", label);
    for (i = 0; i < (*hand).count; i++) {
        if (i == 1 && hide_second) {
            printf("X");
        } else {
            printf("%c", (*hand).cards[i].rank);
        }
        if (i < (*hand).count - 1) {
            printf(", ");
        }
    }
    if (!hide_second) {
        pad = 20 - (*hand).count * 3;
        if (pad < 2) pad = 2;
        printf("%*sValue = %d", pad, "", calculate_hand_count(hand));
    }
    printf("\n");
}

void print_stats(int balance, int bet, int cards_left) {
    // prints balance, current bet, and cards until reshuffle above the board
    printf("  Balance: $%-6d  Bet: $%-6d  Cards until reshuffle: %d\n", balance, bet, cards_left);
}

void print_board(Hand *dealer, Hand *player, int is_dealer_turn, int hide_dealer_second, int balance, int bet, int cards_left) {
    const char *header = is_dealer_turn ? "DEALER'S Turn" : "YOUR Turn";
    clear_screen();
    printf("\n");
    print_stats(balance, bet, cards_left);
    printf("\n  ------ %s ----------------------\n", header);
    print_hand("Dealer", dealer, hide_dealer_second);
    printf("\n");
    print_hand("You", player, 0);
    printf("\n");
}

void print_prompt(int can_double) {
    // can_double is 1 only on the first move (two cards in hand)
    if (can_double) {
        printf("  >>      [H]it  [S]tand  [D]ouble down ? ");
    } else {
        printf("  >>      [H]it  [S]tand ? ");
    }
    printf("\n  ---------------------------------------\n");
}

void print_result(Hand *dealer, Hand *player, int balance, int bet, int cards_left) {
    int pval = calculate_hand_count(player);
    int dval = calculate_hand_count(dealer);

    // reprint board in dealer turn view so result shows below final hands
    print_board(dealer, player, 1, 0, balance, bet, cards_left);
    printf("\n  ----------------------------------------\n");

    if      (dval > 21 || pval > dval) typewrite_result("  You win!");
    else if (pval == dval)             typewrite_result("  Push!");
    else                               typewrite_result("  Dealer wins.");

    printf("  ----------------------------------------\n\n");
}


// game stuff

void deal_initial_cards(Deck *deck, Hand *player, Hand *dealer) {
    clear_screen();
    printf("\n  ------ Dealing -------------------------\n\n");

    printf("  You    :\n");
    animate_deal(player, deal_card(deck), 0);

    printf("\n  Dealer :\n");
    animate_deal(dealer, deal_card(deck), 0);

    printf("\n  You    :\n");
    animate_deal(player, deal_card(deck), 0);

    printf("\n  Dealer :\n");
    animate_deal(dealer, deal_card(deck), 1);

    usleep(400000);
}

// returns 0 = stand, 1 = bust, 2 = double down
int player_turn(Deck *deck, Hand *player, Hand *dealer, int balance, int bet, int cards_left) {
    char choice;
    int val;
    int can_double;

    while (1) {
        can_double = ((*player).count == 2) && (balance >= bet); // can only double if enough balance for extra bet
        print_board(dealer, player, 0, 1, balance, bet, cards_left);
        print_prompt(can_double);
        scanf(" %c", &choice);

        if (choice == 'H' || choice == 'h') {
            add_card(player, deal_card(deck));
            cards_left = cards_left - 1;
            val = calculate_hand_count(player);
            if (val > 21) {
                print_board(dealer, player, 0, 1, balance, bet, cards_left);
                typewrite_result("  Bust! You lose.");
                return 1;
            }
            if (val == 21) {
                break;
            }

        } else if ((choice == 'D' || choice == 'd') && can_double) {
            // double down: one card then stand
            add_card(player, deal_card(deck));
            cards_left = cards_left - 1;
            val = calculate_hand_count(player);
            print_board(dealer, player, 0, 1, balance, bet, cards_left);
            if (val > 21) {
                typewrite_result("  Doubled down -- Bust! You lose.");
                return 1;
            }
            return 2; // signal double down to main so bet is doubled

        } else {
            break; // stand
        }
    }
    return 0;
}

void dealer_turn(Deck *deck, Hand *dealer, Hand *player, int balance, int bet, int cards_left) {
    print_board(dealer, player, 1, 0, balance, bet, cards_left);

    while (calculate_hand_count(dealer) < 17) {
        usleep(500000);
        add_card(dealer, deal_card(deck));
        cards_left = cards_left - 1;
        print_board(dealer, player, 1, 0, balance, bet, cards_left);
    }
}

// returns the bet amount, or -1 if player wants to quit
int ask_for_bet(int balance) {
    char input[32];
    int bet;

    while (1) {
        clear_screen();
        printf("\n  ===== NEW ROUND =========================\n");
        printf("\n  Balance: $%d\n\n", balance);
        printf("  Enter bet amount (min $%d) or [Q] to quit: ", MIN_BET);
        scanf(" %31s", input);

        // check for quit
        if (input[0] == 'Q' || input[0] == 'q') {
            return -1;
        }

        // try to parse as integer
        bet = atoi(input); // atoi returns 0 if not a valid number

        if (bet == 0) {
            printf("\n  ! Invalid input. Enter a number.\n");
            usleep(1200000);
            continue;
        }
        if (bet < MIN_BET) {
            printf("\n  ! Minimum bet is $%d.\n", MIN_BET);
            usleep(1200000);
            continue;
        }
        if (bet > balance) {
            printf("\n  ! You only have $%d. Bet less.\n", balance);
            usleep(1200000);
            continue;
        }

        return bet; // valid bet
    }
}

void reset_hands(Hand *player, Hand *dealer) {
    (*player).count = 0;
    (*dealer).count = 0;
}

int refresh_deck_if_low(Deck *deck) {
    if ((*deck).top > DECK_SIZE - 15) {
        printf("\n  -- Reshuffling deck... --\n");
        usleep(500000);
        init_deck(deck);
        animate_shuffle();
        shuffle_deck(deck);
    }
    return DECK_SIZE - (*deck).top; // return cards remaining after any reshuffle
}


int main(void) {
    srand((unsigned)time(NULL));
    print_main_menu();

    Deck deck;
    int balance = STARTING_BALANCE;
    int bet;
    int cards_left;
    int outcome;    // 0 = stand, 1 = bust, 2 = double down
    int round_bet;  // bet or 2*bet if doubled down
    int pval;
    int dval;
    Hand player;
    Hand dealer;

    init_deck(&deck);
    animate_shuffle();
    shuffle_deck(&deck);

    while (1) {

        // bust out
        if (balance <= 0) {
            clear_screen();
            typewrite_result("  You're broke. The casino wins.");
            printf("\n");
            break;
        }

        bet = ask_for_bet(balance);

        if (bet == -1) {
            break; // player quit
        }

        balance = balance - bet; // deduct bet upfront, add back on win/push
        reset_hands(&player, &dealer);
        cards_left = refresh_deck_if_low(&deck);

        deal_initial_cards(&deck, &player, &dealer);
        cards_left = DECK_SIZE - deck.top;

        if (calculate_hand_count(&player) == 21) {
            balance = balance + bet + (bet * 3/2); // return stake + winnings
            print_board(&dealer, &player, 0, 0, balance, bet, cards_left);
            typewrite_result("  BLACKJACK! You win!");
        } else {
            outcome = player_turn(&deck, &player, &dealer, balance, bet, cards_left);
            cards_left = DECK_SIZE - deck.top;

            round_bet = (outcome == 2) ? bet * 2 : bet; // double the stake if doubled down

            if (outcome == 1) {
                // bust — stake already deducted, nothing to add back
                if (outcome == 2) {
                    balance = balance - bet; // extra bet for double down bust
                }
            } else {
                // stand or double down — dealer plays
                if (outcome == 2) {
                    balance = balance - bet; // deduct the extra double down bet
                }

                dealer_turn(&deck, &dealer, &player, balance, round_bet, cards_left);
                cards_left = DECK_SIZE - deck.top;

                pval = calculate_hand_count(&player);
                dval = calculate_hand_count(&dealer);

                if (dval > 21 || pval > dval) {
                    balance = balance + round_bet * 2; // return stake + winnings
                } else if (pval == dval) {
                    balance = balance + round_bet; // push — return stake only
                }
                // loss — stake already deducted, nothing to add back

                print_result(&dealer, &player, balance, round_bet, cards_left);
            }
        }
    }

    clear_screen();
    printf("\n  Final Balance: $%d  (started with $%d)\n", balance, STARTING_BALANCE);
    printf("\n");
    typewrite("  Thanks for playing. See you next time!\n\n", 25000);

    return 0;
}