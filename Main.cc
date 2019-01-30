/* -*- Mode: C ; indent-tabs-mode: nil ; c-file-style: "stroustrup" -*-

   Rapid Prototyping Floorplanner Project
   Author: Greg Faust

   File:   Main.c     Parse the command line arguments, and call the appropriate functions to do the work.

*/

#include <iostream>
#include <cstdlib>
#include <cmath>
#include "MathUtil.hh"
#include "Floorplan.hh"

void generateTRIPS_Examples()
{
    // First layout the TRIPS core model.
    // We will then use this to generate several different CMP layouts.

    // Define the vertical slide that include the L1 cache and the "Control" block.
    geogLayout * dCacheStack = new geogLayout();
    dCacheStack->addComponentCluster(Control, 1, 4, 10., 1., Top);
    dCacheStack->addComponentCluster(L1, 4, 9, 3., 1., Bottom);

    // Define the remainder of the core.
    geogLayout * CoreCluster = new geogLayout();
    CoreCluster->addComponentCluster(ICache, 5, 1, 10., 1., Left);
    CoreCluster->addComponent(dCacheStack, 1, Left);
    CoreCluster->addComponentCluster(RF, 4, 1, 10., 1., Top);
    CoreCluster->addComponentCluster(Core, 16, 3, 2., 1., Bottom);
    CoreCluster->layout(AspectRatio, 1.);
    ostream& HSOut = outputHotSpotHeader("TRIPS-Core.flp");
    CoreCluster->outputHotSpotLayout(HSOut);
    outputHotSpotFooter(HSOut);

    // Now layout the remainder of the chip.

    // Define the leftmost vertical slide including L2 and the off chip interfaces.
    geogLayout * L2Stack = new geogLayout();
    // No built in types for EBC, C2C, or DMA, so just use strings.
    L2Stack->addComponentCluster("EBC", 1, 3.166, 3., 1., Top);
    L2Stack->addComponentCluster("C2C", 1, 3.166, 3., 1., Bottom);
    L2Stack->addComponentCluster("MC", 2, 3.166, 3., 1., TopBottom);
    L2Stack->addComponentCluster("DMA", 2, 3.166, 3., 1., TopBottom);
    L2Stack->addComponentCluster(L2, 4, 9.5, 2., 1., Center);

    // Define the whole chip.
    geogLayout * WholeChip = new geogLayout();
    WholeChip->addComponent(L2Stack, 1, Left);
    WholeChip->addComponentCluster(L2, 12, 9.5, 2.0, 1., Left);
    WholeChip->addComponent(CoreCluster, 2, TopBottomMirror);

    // Now perform the actual layout.
    bool success = WholeChip->layout(AspectRatio, 1.);

    if (!success) cerr << "Unable to layout specified CMP configuration.";
    else 
    {
        ostream& HSOut = outputHotSpotHeader("TRIPS-Final.flp");
        WholeChip->outputHotSpotLayout(HSOut);
        outputHotSpotFooter(HSOut);
    }
    delete WholeChip;

    // TODO This example never got finished.
    // Now layout the CMP as defined in the initial ISCA paper.
    // With big chunks of L2 plus memory controllers in a + shape in the middle of the chip.
    // With 4 cores in the corners.
    // WholeChip->addComponentCluster(L2, 1, 60, 4, 1, Left);
    // WholeChip->addComponent(CoreCluster, 2, LeftRightMirror);
}

void generateCheckerBoard_Examples()
{
    //////////////////////////////////////////////////////////
    // This section generates the 3 different configurations of 4 alpha chips
    //      from "Microarchitectural Floorplanning for Thermal Management: A Technical Report"
    //      by Karithik Sankaranarayanan, Mircea R. Stan, and Kevin Skadron
    //      Tech Report CS-2005-08, Univ. of Virginia Dept. of Computer Science, May 2005
    /////////////////////////////////////////////////////////

    // Variables used in all the 3 layouts.
    fixedLayout * cont;
    double area;
    geogLayout * top;
    geogLayout * WholeChip;
    bool success;

    // This is the hottest layout with the ALUs and register files near the center of the chip.
    // First load in the alpha chip.
    // It is upside down, so yMirror it.
    cont = new fixedLayout("alpha.flp");
    cont->yMirror = true;
    area = cont->getArea();
    // We need a geo layout to reflect one mirror at a time.
    top = new geogLayout();
    top->addComponent(cont, 2, LeftRightMirror);
    // All the rest can be done at once!!
    WholeChip = new geogLayout();
    WholeChip->addComponentCluster(L2, 2, 4*area, 20, 1, LeftRight);
    WholeChip->addComponentCluster(L2, 2, 2*area, 20, 1, TopBottom);
    WholeChip->addComponent(top, 2, TopBottomMirror);
    
    success = WholeChip->layout(AspectRatio, 1.);

    if (!success) cerr << "Unable to layout specified CMP configuration.";
    else 
    {
        // These are too messy with names.  Just output the shapes.
        setNameMode(false);
        ostream& HSOut = outputHotSpotHeader("CheckCentered.flp");
        WholeChip->outputHotSpotLayout(HSOut);
        outputHotSpotFooter(HSOut);
    }
    delete WholeChip;

    // This is the next hottest layout with the ALUs and register files away from each oterh.
    // First load in the alpha chip.
    cont = new fixedLayout("alpha.flp");
    area = cont->getArea();
    top = new geogLayout();
    top->addComponent(cont, 2, LeftRight);
    top->yMirror = true;
    top->xMirror = true;
    // All the rest can be done at once!!
    WholeChip = new geogLayout();
    WholeChip->addComponentCluster(L2, 2, 4*area, 20, 1, LeftRight);
    WholeChip->addComponentCluster(L2, 2, 2*area, 20, 1, TopBottom);
    WholeChip->addComponent(top, 1, Top);
    WholeChip->addComponent(cont, 2, Left);
    
    success = WholeChip->layout(AspectRatio, 1.);

    if (!success) cerr << "Unable to layout specified CMP configuration.";
    else 
    {
        // These are too messy with names.  Just output the shapes.
        setNameMode(false);
        ostream& HSOut = outputHotSpotHeader("CheckAlternate.flp");
        WholeChip->outputHotSpotLayout(HSOut);
        outputHotSpotFooter(HSOut);
    }
    delete WholeChip;

    // This is the coolest with the alpha chips isolated from each other, surrrounded by cache.
    // First load in the alpha chip.
    // Need to rotate it 180, so use both mirrors.
    cont = new fixedLayout("alpha.flp");
    cont->yMirror = true;
    // cont->xMirror = true;
    area = cont->getArea();

    // We need a geo layout to reflect one mirror at a time.
    top = new geogLayout();
    top->addComponent(cont, 2, LeftRightMirror);
    top->addComponentCluster(L2, 1, area, 1.2, 1, Top);

    // All the rest can be done at once!!
    // First calculate the size of the outside caches.
    // They will account for 7x the area of a core.
    // TODO I should put this code somewhere and reuse it.
    double sideLen = sqrt(area*16.);
    double innerLen = sqrt(area*9.);
    double thickness = sideLen - innerLen;
    double topBotArea = innerLen * thickness;
    double leftRightArea = 7*area - topBotArea;

    // Now we're ready to layout the chip.
    WholeChip = new geogLayout();
    WholeChip->addComponentCluster(L2, 2, leftRightArea/2., 50, 1, LeftRight);
    WholeChip->addComponentCluster(L2, 2, topBotArea/2., 50, 1, TopBottom);
    WholeChip->addComponent(top, 2, TopBottomMirror);
    WholeChip->addComponentCluster(L2, 3, area, 1.2, 1, Center);

    success = WholeChip->layout(AspectRatio, 1.);

    if (!success) cerr << "Unable to layout specified CMP configuration.";
    else 
    {
        // These are too messy with names.  Just output the shapes.
        setNameMode(false);
        ostream& HSOut = outputHotSpotHeader("CheckBoard.flp");
        WholeChip->outputHotSpotLayout(HSOut);
        outputHotSpotFooter(HSOut);
    }
    delete WholeChip;

    //////////////////////////////////////////////////////////
    // This section generates the 4 different configurations of fake cores and caches.
    //      from "Architectural implications of spatial thermal filtering"
    //      by Karthik Sankaranarayanan, Brett H. Meyer, Wei Huang, Robert Ribando, Hossein Haj-Hariri, Mircea R. Stan, Kevin Skadron
    //      Integration, the VLSI Journal, December 2011
    /////////////////////////////////////////////////////////

    // Variables used in all the 4 layouts.
    geogLayout * row;    
    geogLayout * repeat;
    gridLayout * grid;


    // First the 50/50 case.
    row = new geogLayout();
    row->addComponentCluster(L2, 1, 1, 1, 1, Left);
    row->addComponentCluster(Core, 1, 1, 1, 1, Right);
    repeat = new geogLayout();
    repeat->addComponent(row, 2, TopBottom180);
    grid = new gridLayout();
    grid->addComponent(repeat, 16);
        
    success = grid->layout(AspectRatio, 1.);

    if (!success) cerr << "Unable to layout specified CMP configuration.";
    else 
    {
        ostream& HSOut = outputHotSpotHeader("Regular-50.flp");
        grid->outputHotSpotLayout(HSOut);
        outputHotSpotFooter(HSOut);
    }
    delete grid;

    // Now the "Regular 25" case.
    row = new geogLayout();
    row->addComponentCluster(L2, 1, 1.5, 2, 1, Left);
    row->addComponentCluster(Core, 1, .5, 2, 1, Right);
    repeat = new geogLayout();
    repeat->addComponent(row, 2, TopBottom180);
    grid = new gridLayout();
    grid->addComponent(repeat, 16);
        
    success = grid->layout(AspectRatio, 1.);

    if (!success) cerr << "Unable to layout specified CMP configuration.";
    else 
    {
        ostream& HSOut = outputHotSpotHeader("Regular-25.flp");
        grid->outputHotSpotLayout(HSOut);
        outputHotSpotFooter(HSOut);
    }
    delete grid;

    // Now the "Alternate 25" case.
    row = new geogLayout();
    row->addComponentCluster(L2, 1, 1, 2, 1, Left);
    row->addComponentCluster(Core, 1, .5, 2, 1, Left);
    row->addComponentCluster(L2, 1, .5, 2, 1, Right);
    repeat = new geogLayout();
    repeat->addComponent(row, 2, TopBottom180);
    grid = new gridLayout();
    grid->addComponent(repeat, 16);
        
    success = grid->layout(AspectRatio, 1.);

    if (!success) cerr << "Unable to layout specified CMP configuration.";
    else 
    {
        ostream& HSOut = outputHotSpotHeader("Alternate-25.flp");
        grid->outputHotSpotLayout(HSOut);
        outputHotSpotFooter(HSOut);
    }
    delete grid;

    // Now the "Rotated 25" case.
    geogLayout * col = new geogLayout();
    col->addComponentCluster(L2, 1, 1.5, 2, 1, Top);
    col->addComponentCluster(Core, 1, .5, 2, 1, Bottom);
    geogLayout * repeat1 = new geogLayout();
    repeat1->addComponent(col, 1, Left);
    repeat1->addComponentCluster(Core, 1, .5, 2, 1, Top);
    repeat1->addComponentCluster(L2, 1, 1.5, 2, 1, Bottom);

    row = new geogLayout();
    row->addComponentCluster(L2, 1, 1.5, 2, 1, Left);
    row->addComponentCluster(Core, 1, .5, 2, 1, Right);
    geogLayout * repeat2 = new geogLayout();
    repeat2->addComponent(row, 1, Top);
    repeat2->addComponentCluster(Core, 1, .5, 2, 1, Left);
    repeat2->addComponentCluster(L2, 1, 1.5, 2, 1, Right);

    geogLayout * bigRow = new geogLayout();
    bigRow->addComponent(repeat1, 1, Left);
    bigRow->addComponent(repeat2, 1, Right);

    geogLayout * repeat3 = new geogLayout();
    repeat3->addComponent(bigRow, 2, TopBottom180);

    grid = new gridLayout();
    grid->addComponent(repeat3, 4);
        
    success = grid->layout(AspectRatio, 1.);

    if (!success) cerr << "Unable to layout specified CMP configuration.";
    else 
    {
        ostream& HSOut = outputHotSpotHeader("Rotated-25.flp");
        grid->outputHotSpotLayout(HSOut);
        outputHotSpotFooter(HSOut);
    }
    delete grid;
}

void generateMcPAT_ExamplesHelper(int clusterCount, int clusterSize, const char * filename)
{
    // This will be the main workhorse for the McPat examples.
    // We will use this to prove we can have parameterized layouts.
    // We will assume an equal number of cores and caches.
    // We will take in the number of clusters, and the number of core+cache pairs per cluster.
    // We will scale the crossbars appropriately.
    // We will also scale the NoC routers appropriately.
    // Note, there are no memory Controllers.  McPAT examples do.

    // First scale the crossbar.
    double xbarArea = 0;
    double xbarSingleSize = 0.25;
    // 46% scaling as per McPAT paper.
    xbarArea = xbarSingleSize * (.46 * clusterSize * clusterSize);

    // Now scale the NoC component.
    // The 32 cluster McPAT has .52 as the NoC size.
    // These are in a 4x8 grid, so assume size is what is needed for 4 wide bisection bandwidth.
    double baseNoC = .52;
    
    // Get the smaller of the grid dimensions in cluster size.
    int minDim = balanceFactors(clusterCount, .99);
    double NoCArea = baseNoC;
    // Now scale to min dim.
    // 65% scaling factor per McPAT paper.
    if (minDim == 2) NoCArea = baseNoC * 1.65;
    if (minDim == 8) NoCArea = baseNoC / 1.65;

    // Now form the cluster.
    geogLayout * cluster = new geogLayout();
    cluster->addComponentCluster(NoC, 1, NoCArea, 40, 1, Left);
    cluster->addComponentCluster(Core, clusterSize, 1, 4, 1, Top);
    if (clusterSize > 1) cluster->addComponentCluster(CrossBar, 1, xbarArea, 20, 1, Top);
    cluster->addComponentCluster(L2, clusterSize, 1.2, 4, 1, Top);
    
    // Now form the grid
    gridLayout * grid = new gridLayout();
    grid->addComponent(cluster, clusterCount);

    // Since our clusters tend to be wide...
    // Bias the grid to be more components in the y dimension.
    bool success = grid->layout(AspectRatio, 0.999);

    if (!success) cerr << "Unable to layout specified CMP configuration.";
    else 
    {
        ostream& HSOut = outputHotSpotHeader(filename);
        grid->outputHotSpotLayout(HSOut);
        outputHotSpotFooter(HSOut);
    }
    delete grid;
}

void generateMcPAT_Examples()
{
    // Here we will try 
    generateMcPAT_ExamplesHelper(32, 1, "McPAT_32x1.flp");
    generateMcPAT_ExamplesHelper(64, 1, "McPAT_64x1.flp");
    generateMcPAT_ExamplesHelper(16, 4, "McPAT_16x4.flp");
    generateMcPAT_ExamplesHelper(32, 2, "McPAT_32x2.flp");
    generateMcPAT_ExamplesHelper(8, 8, "McPAT_8x8.flp");
}

void generatePenryn45nm ()
{

    // Areas from McPAT.
    double rz_dtlb    = 0.259199;
    double rz_itlb    = 0.0590462;
    double rz_intrat  = 0.59725;
    double rz_flprat  = 0.350662;
    double rz_fl      = 0.0322035;
    double rz_btb     = 0.349484;
    double rz_brp     = 0.153017;
    double rz_instbuf = 0.0278406;
    double rz_instdec = 1.85799;
    double rz_icache  = 0.702441;
    double rz_stq     = 1.06006;
    double rz_ldq     = 0.260806;
    double rz_dcache  = 4.65034;
    double rz_intrf   = 0.336446;
    double rz_flprf   = 0.19163;
    double rz_cplalu  = 0.235435;
    double rz_intiw   = 0.889691;
    double rz_flpiw   = 0.347423;
    double rz_rob     = 0.737129;
    double rz_alu     = 2.82522;
    double rz_fpu     = 4.6585;

    double rpoverhead = 1.1;
    rz_dtlb    *= rpoverhead;
    rz_itlb    *= rpoverhead;
    rz_stq     *= rpoverhead;
    rz_ldq     *= rpoverhead;
    rz_dcache  *= rpoverhead;

    double undiff_ratio = 1.20496;
    rz_dtlb    *= undiff_ratio;
    rz_itlb    *= undiff_ratio;
    rz_intrat  *= undiff_ratio;
    rz_flprat  *= undiff_ratio;
    rz_fl      *= undiff_ratio;
    rz_btb     *= undiff_ratio;
    rz_brp     *= undiff_ratio;
    rz_instbuf *= undiff_ratio;
    rz_instdec *= undiff_ratio;
    rz_icache  *= undiff_ratio;
    rz_stq     *= undiff_ratio;
    rz_ldq     *= undiff_ratio;
    rz_dcache  *= undiff_ratio;
    rz_intrf   *= undiff_ratio;
    rz_flprf   *= undiff_ratio;
    rz_cplalu  *= undiff_ratio;
    rz_intiw   *= undiff_ratio;
    rz_flpiw   *= undiff_ratio;
    rz_rob     *= undiff_ratio;
    rz_alu     *= undiff_ratio;
    rz_fpu     *= undiff_ratio;

    // MMU 
    geogLayout * MMU = new geogLayout();
    MMU->addComponentCluster("Dtlb", 1, rz_dtlb, 20., 1., Left);
    MMU->addComponentCluster("Itlb", 1, rz_itlb, 20., 1., Right);

    // ReN 
    geogLayout * ReN = new geogLayout();
    ReN->addComponentCluster("IntRAT", 1, rz_intrat, 20., 1., Left);
    ReN->addComponentCluster("FlpRAT", 1, rz_flprat, 20., 1., Center);
    ReN->addComponentCluster("FL",     1, rz_fl    , 20., 1., Right);

    // IFU Bottom Left
    geogLayout * IFU_BL = new geogLayout();
    IFU_BL->addComponentCluster("BTB", 1, rz_btb, 20., 1., Left);
    IFU_BL->addComponentCluster("BrP", 1, rz_brp, 20., 1., Center);
    IFU_BL->addComponentCluster("InstBuf", 1, rz_instbuf, 20., 1., Right);

    // IFU left
    geogLayout * IFU_L = new geogLayout();
    IFU_L->addComponentCluster("InstDec", 1, rz_instdec, 20., 1., Top);
    IFU_L->addComponent(IFU_BL, 1, Bottom);

    // IFU
    geogLayout * IFU = new geogLayout();
    IFU->addComponent(IFU_L, 1, Left);
    IFU->addComponentCluster(ICache, 1, rz_icache, 20., 1., Right);

    // core Bottom Right Corner
    geogLayout * core_BR = new geogLayout();
    core_BR->addComponent(ReN, 1, Top);
    core_BR->addComponent(IFU, 1, Center);
    core_BR->addComponent(MMU, 1, Bottom);

    // LSU Top
    geogLayout * LSU_T = new geogLayout();
    LSU_T->addComponentCluster("StQ", 1, rz_stq, 20., 1., Left);
    LSU_T->addComponentCluster("LdQ", 1, rz_ldq, 20., 1., Right);

    // LSU
    geogLayout * LSU = new geogLayout();
    LSU->addComponent(LSU_T, 1, Top);
    LSU->addComponentCluster(DCache, 1, rz_dcache, 20., 1., Bottom);

    // Core Lower Half
    geogLayout * core_LH = new geogLayout();
    core_LH->addComponent(core_BR, 1, Right);
    core_LH->addComponent(LSU, 1, Left);

    // RF in execution block combined with complex ALUs
    geogLayout * EXE_RF = new geogLayout();
    EXE_RF->addComponentCluster("IntRF", 1, rz_intrf, 20., 1., Top);
    EXE_RF->addComponentCluster("FlpRF", 1, rz_flprf, 20., 1., Center);
    EXE_RF->addComponentCluster("CplALU", 1, rz_cplalu, 20., 1., Bottom);

    // IW in exe block
    geogLayout * EXE_IW = new geogLayout();
    EXE_IW->addComponentCluster("IntIW", 1, rz_intiw, 20., 1., Top);
    EXE_IW->addComponentCluster("FlpIW", 1, rz_flpiw, 20., 1., Center);

    // EXE Bottom Right Corner
    geogLayout * EXE_BR = new geogLayout();
    EXE_BR->addComponent(EXE_RF, 1, Left);
    EXE_BR->addComponent(EXE_IW, 1, Center);
    EXE_BR->addComponentCluster("ROB", 1, rz_rob, 20., 1., Right);

    // EXE Right
    geogLayout * EXE_R = new geogLayout();
    EXE_R->addComponentCluster(ALU, 1, rz_alu, 20., 1., Top);
    EXE_R->addComponent(EXE_BR, 1, Bottom);

    // EXE
    geogLayout * EXE = new geogLayout();
    EXE->addComponentCluster("FPU", 1, rz_fpu, 20., 1., Left);
    EXE->addComponent(EXE_R, 1, Right);

    // Core area
    geogLayout * core = new geogLayout();
    // The input parameters are: Type, Count, Area, MaxAspectRatio, MinAspectRatio, Hint
    core->addComponent(EXE, 1, Center);
    core->addComponent(core_LH, 1, Bottom);

    bool success = core->layout(AspectRatio, 1);
    if (!success) cout << "Unable to layout specified CMP configuration.";
    else 
    {
        setNameMode(false);
        ostream& HSOut = outputHotSpotHeader("Penryn45.flp");
        core->outputHotSpotLayout(HSOut);
        outputHotSpotFooter(HSOut);
        setNameMode(true);
    }
    delete core;
}

void generate8corePenrynCMP()
{
  // Areas from McPAT.
  double rz_noc     = 0.95931;
  double rz_mc      = 3.55434;
  double rz_l2      = 6.70483;

  // Load in the core into a fixed layout.
  fixedLayout * core = new fixedLayout("Penryn45.flp", 22./45.);

  // Lower half of ccore, includes L2, NoC and MC
  // We create MC for every chip, but only assign power to L2 chunks that do have MC
  geogLayout * L2chunk = new geogLayout();
  // The input parameters are: Type, Count, Area, MaxAspectRatio, MinAspectRatio, Hint
  L2chunk->addComponentCluster(L2,  1, rz_l2, 20., 1., Right);
  L2chunk->addComponentCluster(NoC, 1, rz_noc, 50., 1., Left);

  // Now just create all the rest at once.
  geogLayout * chip = new geogLayout();  
  chip->addComponentCluster("MC",  8, rz_mc/2, 20., 1., TopBottom);
  //chip->addComponentCluster("Core", 8, rz_core, 20., 1., TopBottom);
  chip->addComponent(core, 8, TopBottomMirror);
  chip->addComponent(L2chunk, 8, Center);
  bool success = chip->layout(AspectRatio, 1);
  if (!success) cout << "Unable to layout specified CMP configuration.";
  else 
  {
      ostream& HSOut = outputHotSpotHeader("CMP8Core.flp");
      chip->outputHotSpotLayout(HSOut);
      outputHotSpotFooter(HSOut);
  }
  delete chip;
}

void generateFixedLayout_Example()
{
  // Here is an example of loading in an existing hotspot floorplan into a "fixed" layout.
  // Take an alpha chip as input, and relayout at various aspect ratios.
  // Load in the alpha chip in hotspot format.
  fixedLayout * cont = new fixedLayout("alpha.flp");
  // Output the chip at its original layout
  ostream& HSOut = outputHotSpotHeader("alphadup.flp");
  cont->outputHotSpotLayout(HSOut);
  outputHotSpotFooter(HSOut);
  // Change the layout appect ratio to 2/1
  cont->layout(AspectRatio, 2);
  ostream& HSOut2 = outputHotSpotHeader("alphaTwo.flp");
  cont->outputHotSpotLayout(HSOut2);
  outputHotSpotFooter(HSOut2);
  // Change the aspect ratio to 1/2
  cont->layout(AspectRatio, .5);
  ostream& HSOut3 = outputHotSpotHeader("alphaOneHalf.flp");
  cont->outputHotSpotLayout(HSOut3);
  outputHotSpotFooter(HSOut3);
  delete cont;
}

int main(int argc, char* argv[])
{
  string usageString = "Usage:\nArchFP [-h] [-v]\n-h     Print out this help information.\n-v     Output verbose layout information to stdout.\n"
    "ArchFP does not take in declarative specifications for floorplans.\n"
    "To produce a different floorplan, please edit Main.cc and recompile.\n";
  string copyrightString = "ArchFP: A pre-RTL rapid prototyping floorplanner.\nAuthor: Greg Faust (gf4ea@virginia.edu).\n";

  for (int x = 1; x < argc; x++)
    {
      string arg = string(argv[x]);
      if (arg == "-h")
        {
          cout << copyrightString;
          cout << usageString;
          return 0;
        }
      else if (arg == "-v")
        {
          verbose = true;
        }
      else
        {
          cout << copyrightString;
          cout << usageString;
          return 1;
        }
    }
  cout << copyrightString;

  ///////////////////////////////////////////////////////
  // Look at these subroutines above for examples of how to build floorplans using this tool.
  //////////////////////////////////////////////////////

  generateTRIPS_Examples();
  generateCheckerBoard_Examples();
  generateMcPAT_Examples();
  generateFixedLayout_Example();

  // Example code to generate floorplan in SoC2012 Open Source Tool submission.
  generatePenryn45nm();
  generate8corePenrynCMP();

  return 0;
}

