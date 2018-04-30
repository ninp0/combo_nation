#include <AFMotor.h>

// TODO: Config this in the Menu
int MAX_COMBO_NUM = 40;
int COMBO_LENGTH = 3;
int ONE_REVOLUTION_ACTUATOR = 200;
int ONE_REVOLUTION_MOTOR = 200; // Steps in one revolution
int LATCH_PIN = A2;

// Stepper logic
AF_Stepper actuator(ONE_REVOLUTION_ACTUATOR, 1); // Linear Actuator for engaging latch following combo attempt
AF_Stepper motor(ONE_REVOLUTION_MOTOR, 2); // Motor on Channel 2 for better physical cable mgmt
double STEP_ONE = ((double)ONE_REVOLUTION_MOTOR * 2) / (double)MAX_COMBO_NUM;

void setup() {  
  pinMode(LATCH_PIN, OUTPUT);
  Serial.begin(9600);
  Serial.print(">>> initializing auto-dialer motor...");
  actuator.setSpeed(150.0); // 50 rpm 
  motor.setSpeed(150.0); // 150 rpm for maximum accuracy w/ NEMA 16 12V Stepper
  Serial.println("complete.");
}

void screen_menu(int menu_option) {
  int line_count = 3;
  for(int i = 0; i < line_count; i++) {
    Serial.println(); 
  }

  Serial.println(" ______________________________________________");
  Serial.println("|  _  _ __ |_  _    __  _ _|_ o  _ __"); 
  Serial.println("| (_ (_)||||_)(_) o | |(_| |_ | (_)| |");        
  Serial.println("|");
  Serial.println("|               .-^^-.");
  Serial.println("|              / .--. \"");
  Serial.println("|             / /   \\  \" ");
  Serial.println("|             | |    | |");
  Serial.println("|             | |.-\"\"-.|");
  Serial.println("|            ///`.::::.`;");
  Serial.println("|           ||| ::/  \\:: ;");
  Serial.println("|           ||; ::\\_ /:: ;");
  Serial.println("|            \\\\  ':::::'/");
  Serial.println("|             `'':-..-'`");
  Serial.println("|");  
  Serial.println("|______________________________________________");
  Serial.println();
  Serial.println();
  Serial.println("INFO >>> ENSURE COMBINATION DIAL IS CURRENTLY 0");
  Serial.println();
  Serial.println();
  Serial.println("MENU >>>");
  Serial.println(" ______________________________________________");
  Serial.println("|  1. dial calibration");
  Serial.println("|  2. demo");
  Serial.println("|  3. enter combination manually");  
  Serial.println("|  4. robo-dialing via serial (slower but diverse)");
  Serial.println("|  5. robo-dialing via locale (faster but limited)");
  Serial.println("|  6. robo-manipulation (fastest)");
  Serial.println("|  7. reset combination gates");
  Serial.println("|  8. open latch");
  Serial.println("|______________________________________________");
  Serial.println();
  Serial.println();
  Serial.print("option # ");
  Serial.write(menu_option);
  Serial.println();
  
  motor.release(); // Release torque on motor to give it a break and allow manual re-calibration
  clear_serial_input();
} 

void dial_calibration() {
  Serial.println(">>> MODE: dial calibration");
  Serial.println(">>> initializing dial calibration mode");
  for (int this_combo = 0; this_combo <= MAX_COMBO_NUM; this_combo++) {
    Serial.print(">>> moved dial to #");
    Serial.println(this_combo);
    counter_clockwise(this_combo);
    delay(2500);
    clockwise(this_combo); // reset for next combo
    delay(500);
  }  
}

void demo() {
  Serial.println(">>> MODE: demo");
  int combo_sequence[] = {28, 14, 4};
  autodial_combo_sequence(combo_sequence);
}

void enter_combo_manually() {
  Serial.print(">>> MODE: enter combination manually: ");
  get_serial_combo();
}

void robodial_via_serial() {
  Serial.println(">>> MODE: robo-dial via serial");
  while(true) { // TODO: Exit to menu when complete.
    get_serial_combo(); 
  }
}

void reset_gates() {
  // Reset Combination Gates
  clockwise((MAX_COMBO_NUM * 3));
  delay(333);
  motor.release();
  delay(333);
}
// End of Menu Functions

// Funcions required to perform properly
void clockwise(double combo_steps) {
  motor.step((combo_steps * STEP_ONE), FORWARD, INTERLEAVE);
}

void counter_clockwise(double combo_steps) {
  motor.step((combo_steps * STEP_ONE), BACKWARD, INTERLEAVE); 
}

void open_latch() {
  motor.release();
  Serial.print("latch attempt...");
  //digitalWrite(LATCH_PIN, HIGH);
  //delay(333);
  //digitalWrite(LATCH_PIN, LOW);
  // TODO: Figure out when we're not opening so we can stop
  actuator.step(400, BACKWARD, DOUBLE); //Down
  actuator.step(400, FORWARD, DOUBLE); // Up
  actuator.release();
  Serial.println("complete."); 
}

void display_combination(int *combo_sequence) {
  Serial.print(">>> performing combination permutation: ");
  
  for (int i  = 0; i < COMBO_LENGTH; i++) {      
    Serial.print(" ");
    Serial.print(combo_sequence[i]);
  }
  
  Serial.println();
}

void clear_serial_input() {
  while (Serial.available()) {
    Serial.read();
  }
}

void get_serial_combo() {
  String serial_single_combo;
  while (Serial.available() == 0) {} // Suspect bug here for input
  serial_single_combo = Serial.readString();
  Serial.println(serial_single_combo);
  
  int i;
  int offset = 0;
  int combo_sequence[COMBO_LENGTH];
  int combo_index = 0;
  for (i = 0; i < serial_single_combo.length(); i++) {
    if (serial_single_combo[i] == ',') {
      // TODO: Convert to function
      String this_combo_permutation;
      for (int j = offset; j < i; j++) {
        //Serial.print(serial_single_combo[j]);
        this_combo_permutation += serial_single_combo[j];
      } 
      //Serial.println(this_combo_permutation); 
      char this_combo_num[this_combo_permutation.length() + 1];
      this_combo_permutation.toCharArray(this_combo_num, sizeof(this_combo_num));
      combo_sequence[combo_index] = atoi(this_combo_num);
      // End of function
      
      combo_index++;
      offset = i + 1;
    } 
  }

  // TODO: Convert to function
  String this_combo_permutation;
  for (int j = offset; j < i; j++) {
    //Serial.print(serial_single_combo[j]);
    this_combo_permutation += serial_single_combo[j];
  } 
  //Serial.println(this_combo_permutation);
  char this_combo_num[this_combo_permutation.length() + 1];
  this_combo_permutation.toCharArray(this_combo_num, sizeof(this_combo_num));
  combo_sequence[combo_index] = atoi(this_combo_num);  
  // End of function
  
  autodial_combo_sequence(combo_sequence);
}

void autodial_combo_sequence(int *combo_sequence) {
  // Display Combination
  display_combination(combo_sequence);
  
  // Reset Combination Gates
  reset_gates();
  
  // Begin Combination Permutation
  int step_sequence[COMBO_LENGTH];
  
  for (int i = 0; i < COMBO_LENGTH; i++) {
    if (i % 2 == 0) {
      if (i == 0) {
        step_sequence[i] = MAX_COMBO_NUM - combo_sequence[i];
      } else {
        if (combo_sequence[i - 1] <= combo_sequence[i]) {
          step_sequence[i] = combo_sequence[i -1] + (MAX_COMBO_NUM - combo_sequence[i]);
        } else {
          step_sequence[i] = combo_sequence[i - 1] - combo_sequence[i];  
        }
      }
      clockwise(step_sequence[i]);
    } else {
      // This still needs work if COMBO_LENGTH > 3
      step_sequence[i] = MAX_COMBO_NUM + step_sequence[i - 1] + combo_sequence[i];
      counter_clockwise(step_sequence[i]);
    }
    delay(333);
  }  
  
  // TODO: Actuator (push/pull solenoid) for pulling on shackle or handle
  open_latch();
  
  // Reset Back to Zero
  delay(999);
  clockwise(combo_sequence[COMBO_LENGTH - 1]); // Last index in the array
  motor.release();
}

// TODO: Create modes:
// debug
// bruteforce
// date/reverse date guessing
// sequential guessing (e.g. 1234, 5678, etc.)
// only odd number guessing
// only even number guessing
// matching numbers guessing (e.g. 000, 111, etc)
// combination model which reference stop gate algorithm number/etc)
// stop gates algorithm deciphering
// manipulation leveraging accelerometer to detect minute vibrations 
void loop() {  
  screen_menu(0);
  while (Serial.available() == 0) {}
  String serial_in = Serial.readString();
  char this_char[serial_in.length() + 1];
  serial_in.toCharArray(this_char, sizeof(this_char));
  int MENU_OPTION = atoi(this_char);
    
  Serial.print("menu option chosen: ");
  Serial.println(MENU_OPTION);
  // Which menu option?
  switch(MENU_OPTION) {
    case 1: { // Option 1
      screen_menu(MENU_OPTION);
      dial_calibration();
      break;
    }
    case 2: { // Option 2
      screen_menu(MENU_OPTION);
      demo();
      break;
    }
    case 3: { // Option 3
      screen_menu(MENU_OPTION);
      enter_combo_manually();
      break;
    }
    case 4: { // Option 4
      screen_menu(MENU_OPTION);
      robodial_via_serial();
      break;
    }
    case 5: { // Option 5
      // TODO: built fast permutation brute-force algorithm w/ char repetition
      Serial.println("Coming soon...");
      delay(3000);
      break;
    }
    case 6: { // Option 6
      Serial.println("Coming soon...");
      delay(3000);
      break;
    }
    case 7: { // Option 7
      reset_gates();
      break;
    }
    case 8: {
      open_latch();
      break; 
    }
    default: {
    Serial.print("invalid menu option!");
      delay(3000);
      //screen_menu(0);
    }
  }
}

