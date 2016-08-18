
class chip8{
unsigned short opcode;
unsigned char memory[4097];
unsigned char V[16];
unsigned short I;
unsigned short pc;
unsigned short sp;
unsigned char delay_timer;
unsigned char sound_timer;
unsigned short stack[16];
unsigned short x ;
unsigned short y ;
unsigned short height ;
unsigned short pixel;
bool drawFlag;
unsigned char gfx[64 * 32];
public:
	int getGfx(int i);
    bool getDrawFlag();
	void initialize();
	void emulateCycle();
	unsigned char key[16];
	bool loadApplication(const char * fname);
};
unsigned char chip8_fontset[80] =
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
void chip8::initialize()
 {
	pc     =0x200;	// program counter
	//starts at 200 because that is what the program expects
	opcode =0;		// reset curent opcode
	I	   =0;		// reset index counter
	sp     =0;		//reset stack pointer

	// clear display
	// clear stack
	// clear registers V0-VF
	// clear memory

	//load fontset
	  for(int i = 0; i < 80; ++i){
		memory[i] = chip8_fontset[i];
	  }

 };
void chip8::emulateCycle()
{
	drawFlag=false;
  // Fetch Opcode
	opcode=  (memory[pc] << 8) | memory[pc+1];
  // Decode Opcode
	switch(opcode & 0xF000){

		case 0x0000:
			if( (opcode & 0x00FF) == 0x00EE){// return from subroutine
				//call 0x00EE
				sp--;
				pc=stack[sp];//decrement stack and load old programcounter
				pc +=2;//increment by 2 to skip called instruction
				break;
			}
			if ((opcode & 0x00ff) == 0x00E0){//clear screen
				//call 0x00e0
				memset(gfx, 0, sizeof(gfx));
				drawFlag =true;
				pc+=2;
				break;
			}
			// call 0NNN
			break;
		case 0x1000://jump to address 
			pc = opcode & 0x0FFF;
			//call opcode
			break;
		case 0x2000://call subroutine
			  stack[sp] = pc;
			  ++sp;
              pc = opcode & 0x0FFF;
			break;
		case 0x3000:// 0x3XNN: Skips the next instruction if VX equals NN
			//call opcode
			if (V[(opcode & 0xF00) >> 8] == (opcode & 0x0FF)){
				pc+=4;
			}
			else{
				pc+=2;
			}
			break;
		case 0x4000: //skips next instruction if VX doesnt equal NN
			//call opcode
			if (V[(opcode & 0xF00) >> 8] != (opcode & 0x0FF)){
				pc+=4;
			}
			else{
				pc+=2;
			}
			break;
		case 0x5000: // skips next instruction if Vx = Vy
			//call opcode
			if (V[(opcode & 0xF00) >> 8] == V[(opcode & 0x0F0) >> 4]){
				pc+=4;
			}
			else{
				pc+=2;
			}
			break;
		case 0x6000: //sets Vx to NN
			//call opcode
			V[(opcode & 0x0F00) >> 8 ] = opcode & 0x00FF;
			pc+=2;
			break;
		case 0x7000:// Adds NN to Vx
			//call opcode
			V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
			pc+=2;
			break;
		case 0x8000:
			if (( opcode & 0x000F) == 0x0000){//sets value of Vx to Vy
				//do stuff
				V[(opcode & 0x0F00) >> 8 ] =V[(opcode & 0x00F0) >> 4 ];
				pc+=2;
				break;
			}
			if (( opcode & 0x000F) == 0x0001){// Sets Vx to Vx or Vy
				//do stuff
				V[(opcode & 0x0F00) >> 8 ] |= V[(opcode & 0x00F0) >> 4 ];
				pc+=2;
				break;
			}
			if (( opcode & 0x000F) == 0x0002){// sets Vx to Vx & Vy
				//do stuff
				V[(opcode & 0x0F00) >> 8 ] &= V[(opcode & 0x00F0) >> 4 ];
				pc+=2;
				break;
			}
			if (( opcode & 0x000F) == 0x0003){// sets Vx to Vx xor Vy
				//do stuff
				V[(opcode & 0x0F00) >> 8 ] ^= V[(opcode & 0x00F0) >> 4 ];
				pc+=2;
				break;
			}
			if (( opcode & 0x000F) == 0x0004){//adds Vy to Vx
				//do stuff
				if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])){
					V[0xF] = 1;
				}
				else {
					V[0xF] = 0;
				}
				V[(opcode & 0x0F00) >> 8] += V[( opcode & 0x00F0) >> 4];
				pc +=2;
				break;
			}
			if (( opcode & 0x000F) == 0x0005){//Vy is Subtracted from Vx
				//do stuff
				if (V[(opcode & 0x0F00) >> 8] < V[(opcode & 0x00F0) >> 4]){
					V[0xF] = 0;
				}
				else{
					V[0xF] =1;
				}
				V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
				pc+=2;
				break;
			}

			if (( opcode & 0x000F) == 0x0006){
				//shifts Vx right by one Vf is set to the value of the least signifcant
				//bit of Vx before the shift
				V[0xF] = (V[(opcode & 0x0F00) >> 8] & 0x1 );
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] >>1;
				pc+=2;
				break;
			}
			if (( opcode & 0x000F) == 0x0007){//sets Vx to Vy - Vx  Vf tracks a borrow
				//do stuff
				if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]){
					V[0xF] = 1;
				}
				else{
					V[0xF] =0;
				}
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
				pc+=2;
				break;
			}
			if (( opcode & 0x000F) == 0x000E){// shifts Vx left by one Vf is set to most sig bit
				//do stuff
				V[0xF] = (V[(opcode & 0x0F00) >> 8] >> 7);
				V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] << 1;
				pc+=2;
				break;
			}
			;
		case 0x9000:// skips the next instruction if Vx doesn't equal Vy
			//dostuff
			if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]){
				pc+=4;
			}
			else{
				pc+=2;
			}
			break;

		case 0xA000://sets I to address NNN
			//dostuff
			I = opcode & 0x0FFF;
			pc+=2;
			break;
		case 0xB000://jumps to address NNN + V[0]
			//dostuff
			pc = (opcode & 0x0FFF) + V[0];
			break;
		case 0xC000:// sets Vx to NN &
			//dostuff
			V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
			pc += 2;
			break;
		case 0xD000://draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
			//VF is set to 1 if any pixels are changed from set to unset
			 x = V[(opcode & 0x0F00) >> 8];
			 y = V[(opcode & 0x00F0) >> 4];
			 height = opcode & 0x000F;

			V[0xF] = 0;
			for (int yline = 0; yline < height; yline++)
			{
				pixel = memory[I + yline];
				for(int xline = 0; xline < 8; xline++)
				{
					if((pixel & (0x80 >> xline)) != 0)
					{
						if(gfx[(x + xline + ((y + yline) * 64))] == 1){
							V[0xF] = 1;  
						}
						gfx[x + xline + ((y + yline) * 64)] ^= 1;
					}
				}
			}

			drawFlag = true;
			pc += 2;
			break;
		case 0xE000:
			//skips next instruction is key stored in Vx is pressed
			if (( opcode & 0x00FF) == 0x009E){
				      if(key[V[(opcode & 0x0F00) >> 8]] != 0){
						  pc += 4;
					  }
				      else{
						  pc += 2;
					  }
			break;
			}
			//skips next instruction is key stored in Vx isnt pressed
			if (( opcode & 0x00FF) ==0x00A1){
				if(key[V[(opcode & 0x0F00) >>8]] ==0){
					pc+=4;
				}
				else{
					pc+=2;
				}
				break;
			}
			;
		case 0xF000:
			if ((opcode & 0x000F) == 0x0007){//sets Vx to the delay timer
				//do stuff
				V[(opcode & 0x0F00) >> 8] = delay_timer;
				pc+=2;
				break;
			}
			if ((opcode & 0x000F) == 0x000A){
				//a key press is waited and then stored in vx
				for (int i=0;i<16;i++){
					if( key[i] !=0){
						V[(opcode &0x0F00) >>8] = i;
						pc+=2;
						break;
					}
					if (i == 15){
						return;
					}
				}
				break;
			}
			if ((opcode & 0x00FF) == 0x0015){//sets the delay timer to Vx
				//do stuff
				delay_timer = V[(opcode & 0x0F00) >> 8] ;
				pc+=2;
				break;
			}
			if ((opcode & 0x00FF) == 0x0018){//Sets the sound timer to Vx
				//do stuff
				sound_timer = V[(opcode & 0x0F00) >> 8] ;
				pc+=2;
				break;
			}
			if ((opcode & 0x00FF) == 0x001E){//adds Vx to I
				//do stuff
				I +=V[(opcode & 0x0F00) >> 8] ;
				pc+=2;
				break;
			}
			if ((opcode & 0x00FF) == 0x0029){
				//Sets I to the location of the sprite for the character in VX. 
				//Characters 0-F (in hexadecimal) are represented by a 4x5 font.
				//do stuff
				I = V[(opcode & 0x0F00) >> 8] * 0x5;
				pc += 2;
				break;
			}
			if ((opcode & 0x00FF) == 0x0033){//stores the binary coded decimal of Vx
				//do stuff
				memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
				memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
				memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
				pc += 2;
				break;
			}
			if ((opcode & 0x00FF) == 0x0055){//stores V0 toVx in memory
				//do stuff
				for(int x=0; x<= ((opcode & 0x0F00) >>8); x++){
					memory[I +x] = V[x];
				}
				I += ((opcode & 0x0F00) >> 8) + 1;
				pc+=2;
				break;
			}
			if ((opcode & 0x00FF) == 0x0065){//fills V0 to Vx from memory 
				//do stuff
				for(int x=0; x<= ((opcode & 0x0F00) >>8); x++){
					 V[x] = memory[I +x];
				}
				I += ((opcode & 0x0F00) >> 8) + 1;
				pc+=2;
				break;
			}
			;
	default:
		pc+=2;
	}
 
  // Update timers
	if(delay_timer >0){
		delay_timer--;
	}
	if(sound_timer >0){
		if( sound_timer==1){
			Beep(100,100);
		}
		sound_timer--;
	}
	}
bool chip8::loadApplication(const char * filename)
{
	printf("Loading: %s\n", filename);
		
	// Open file
	FILE * pFile;
	fopen_s(&pFile,filename, "rb");
	if (pFile == NULL)
	{
		fputs ("File error", stderr); 
		return false;
	}

	// Check file size
	fseek(pFile , 0 , SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	printf("Filesize: %d\n", (int)lSize);
	
	// Allocate memory to contain the whole file
	char * buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL) 
	{
		fputs ("Memory error", stderr); 
		return false;
	}

	// Copy the file into the buffer
	size_t result = fread (buffer, 1, lSize, pFile);
	if (result != lSize) 
	{
		fputs("Reading error",stderr); 
		return false;
	}

	// Copy buffer to Chip8 memory
	if((4096-512) > lSize)
	{
		for(int i = 0; i < lSize; ++i)
			memory[i + 512] = buffer[i];
	}
	else
		printf("Error: ROM too big for memory");
	
	// Close file, free buffer
	fclose(pFile);
	free(buffer);

	return true;
}
bool chip8::getDrawFlag(){
	return drawFlag;
}
int chip8::getGfx(int i){
	return gfx[i];
}