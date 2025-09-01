/** \file
*   Apply selected gain to audio input.
*/
string name = "AutoLevel2";
string description = "Compressor";

// === Controls ===
array<double> inputParameters(4); // 4 knobs
array<string> inputParametersNames = {"Input", "Output", "Gamma", "Theta"};

// For Gamma/Theta UI knobs, use 0 → 1 and map internally
array<double> inputParametersMin = {0.00025, 1, 0.0, 0.0};
array<double> inputParametersMax = {3, 10, 1.0, 1.0};

// Defaults are normalized knob positions (calculated from desired start value)
array<double> inputParametersDefault = {
    1,                       // Input
    1,                       // Output
    log(0.008 / 0.00001) / log(0.1 / 0.00001), // Gamma knob position
    log(0.001 / 0.00001) / log(0.1 / 0.00001)  // Theta knob position
};

// === Variables ===
double input;
double output;
double gamma; // attack time (mapped value)
double theta; // release time (mapped value)

// Range constants for mapping
const double gammaMin = 0.00001;
const double gammaMax = 0.1;
const double thetaMin = 0.00001;
const double thetaMax = 0.1;

// Extra variables
double a, a2, b = 1, b2 = 1, zm = 1, zm2 = 1;

void processSample(array<double>& ioSample)
{
    // === APPLY INPUT GAIN ===
    double inputL = ioSample[0] * input;
    double inputR = ioSample[1] * input;

    // === YOUR COMPRESSOR LOGIC ===
   a=((abs(ioSample[0]*2)*input));    
           b=(1-gamma)*((b)+((abs(zm-b))))+gamma*(pow((a*20),2))/((zm)+0.0000000001);
           zm=(1-theta)*((b)+(abs(zm-b)))+theta*pow(abs(zm),abs(abs(b)/(zm+0.0000001)));
    a2=((abs(ioSample[1]*2)*input));
            b2=(1-gamma)*((b2)+((abs(zm2-b2))))+gamma*(pow((a2*20),2))/((zm2)+0.0000000001);
           zm2=(1-theta)*((b2)+abs(zm2-b2))+theta*pow(abs(zm2),abs(abs(b2)/(zm2+0.0000001)));
    ioSample[0] = ((ioSample[0])/(zm*0.5 + zm2*0.5)) * output;
    ioSample[1] = ((ioSample[1])/(zm*0.5 + zm2*0.5)) * output;
}

void updateInputParameters()
{
    input  = inputParameters[0];
    output = inputParameters[1];

    // Map Gamma knob (0-1) to log scale
    gamma = gammaMin * pow(gammaMax / gammaMin, inputParameters[2]);

    // Map Theta knob (0-1) to log scale
    theta = thetaMin * pow(thetaMax / thetaMin, inputParameters[3]);
}
