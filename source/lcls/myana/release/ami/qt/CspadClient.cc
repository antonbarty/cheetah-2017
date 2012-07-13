#include "ami/qt/CspadClient.hh"
#include "ami/qt/ChannelDefinition.hh"
#include "ami/data/ConfigureRequest.hh"
#include "ami/data/EntryImage.hh"
#include "ami/data/Entry.hh"
#include "ami/data/DescEntry.hh"

#include <QtGui/QCheckBox>
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>

using namespace Ami::Qt;

CspadClient::CspadClient(QWidget* w,const Pds::DetInfo& i, unsigned u) :
  ImageClient(w, i, u)
{
  { QPushButton* pedB = new QPushButton("Write Pedestals");
    addWidget(pedB);
    connect(pedB, SIGNAL(clicked()), this, SLOT(write_pedestals())); }

  addWidget(_spBox = new QCheckBox("Suppress\nBad Pixels"));
  addWidget(_fnBox = new QCheckBox("Correct\nCommon Mode"));
}

CspadClient::~CspadClient() {}

void CspadClient::save(char*& p) const
{
  XML_insert(p, "ImageClient", "self", ImageClient::save(p) );
  XML_insert(p, "QCheckBox", "_fnBox", QtPersistent::insert(p,_fnBox->isChecked()) );
  XML_insert(p, "QCheckBox", "_spBox", QtPersistent::insert(p,_spBox->isChecked()) );
}

void CspadClient::load(const char*& p)
{
  XML_iterate_open(p,tag)
    if (tag.element == "ImageClient")
      ImageClient::load(p);
    else if (tag.name == "_fnBox")
      _fnBox->setChecked(QtPersistent::extract_b(p));
    else if (tag.name == "_spBox")
      _spBox->setChecked(QtPersistent::extract_b(p));
  XML_iterate_close(CspadClient,tag);
}

void CspadClient::_configure(char*& p, 
                             unsigned input, 
                             unsigned& output,
                             ChannelDefinition* ch[], 
                             int* signatures, 
                             unsigned nchannels)
{
  unsigned o = 0;
  if (_fnBox->isChecked()) o |= 2;
  if (_spBox->isChecked()) o |= 1;
  ConfigureRequest& req = *new(p) ConfigureRequest(input,o);
  p += req.size();

  ImageClient::_configure(p,input,output,ch,signatures,nchannels);
}

void CspadClient::_setup_payload(Cds& cds)
{
#if 0
  if (_input_entry) {
    //  Multiple display clients compete for these options
    for(unsigned i=0; i<NCHANNELS; i++) {
      unsigned signature = _channels[i]->signature();
      const Entry* e = cds.entry(signature);
      if (e) {
        unsigned o = e->desc().options();

        unsigned oo(0);
        if (_fnBox->isChecked()) oo |= 2;
        if (_spBox->isChecked()) oo |= 1;
        if (o != oo) {
          printf("CspadClient::setup_payload options %x -> %x\n",oo,o);
        }

        _fnBox->setChecked(o&2);
        _spBox->setChecked(o&1);
        break;
      }
    }
  }
#endif
  ImageClient::_setup_payload(cds);
}      

#include "pdsdata/cspad/Detector.hh"

enum Rotation { D0, D90, D180, D270, NPHI=4 };
static const Rotation _tr[] = {  D0  , D90 , D180, D90 ,
                                 D90 , D180, D270, D180,
                                 D180, D270, D0  , D270,
                                 D270, D0  , D90 , D0 };

void CspadClient::write_pedestals()
{
  QString name;
  unsigned signature=-1UL;

  for(int i=0; i<NCHANNELS; i++)
    if (_channels[i]->is_shown()) {
      name = _channels[i]->name();
      signature = _channels[i]->output_signature();
    }

  QString msg = QString("Write a new pedestal file \nwith the offsets in %1?").arg(name);

  QMessageBox box;
  box.setWindowTitle("Write Pedestals");
  box.setText(msg);
  box.addButton(QMessageBox::Cancel);

  QPushButton* prodB = new QPushButton("Prod");
  QPushButton* testB = new QPushButton("Test");

  box.addButton(testB,QMessageBox::AcceptRole);

  if (QString(getenv("HOME")).endsWith("opr"))
    box.addButton(prodB,QMessageBox::AcceptRole);

  if (box.exec()==QMessageBox::Cancel)
    ;
  else {

    const Ami::EntryImage& entry = *static_cast<const Ami::EntryImage*>(_cds.entry(signature));
    const unsigned nframes = entry.desc().nframes();
    //
    //  Load pedestals
    //
    double** _off = new double*[nframes];
    for(unsigned s=0; s<nframes; s++)
      _off[s] = new double[Pds::CsPad::MaxRowsPerASIC*Pds::CsPad::ColumnsPerASIC*2];

    std::string oname;
    std::string nname;

    char tbuf[32];
    sprintf(tbuf,"%08x.dat",entry.desc().info().phy());
    oname = std::string("ped.") + tbuf;

    FILE* f;
    if (box.clickedButton() == testB)
      f = fopen(oname.c_str(),"r");
    else {
      oname = std::string("/reg/g/pcds/pds/cspadcalib/ped.") + tbuf;
      f = fopen(oname.c_str(),"r");
    }

    if (f) {

      //  read pedestals
      size_t sz = 8 * 1024;
      char* linep = (char *)malloc(sz);
      char* pEnd;

      for(unsigned s=0; s<nframes; s++) {
        double* off = _off[s];
        for(unsigned col=0; col<Pds::CsPad::ColumnsPerASIC; col++) {
          getline(&linep, &sz, f);
          *off++ = strtod(linep,&pEnd);
          for (unsigned row=1; row < 2*Pds::CsPad::MaxRowsPerASIC; row++)
            *off++ = strtod(pEnd, &pEnd);
        }
      }    

      free(linep);

      //  rename current pedestal file

      printf("Loaded pedestals from %s\n",oname.c_str());

      time_t t = time(0);
      strftime(tbuf,32,"%Y%m%d_%H%M%S",localtime(&t));
    
      nname = oname + "." + tbuf;

      rename(oname.c_str(),nname.c_str());
    }
    else
      printf("Failed to load pedestals\n");

    if (f)
      fclose(f);

    bool fail=false;
    FILE* fn = fopen(oname.c_str(),"w");
    if (!fn) {
      msg = QString("Unable to write new pedestal file %1").arg(oname.c_str());
      fail=true;
    }
    else if ((entry.desc().info().device()==Pds::DetInfo::Cspad2x2 && nframes != 2) ||
             (entry.desc().info().device()==Pds::DetInfo::Cspad    && nframes != 32)) {
      msg = QString("Failed.  Must readout entire detector.  Check configuration.");
      fail=true;
      fclose(fn);
    }
    else {
      for(unsigned i=0; i<nframes; i++) {
        double* off = _off[i];
        const SubFrame& frame = entry.desc().frame(i);
        double dn   = entry.info(EntryImage::Normalization);
        double doff = entry.info(EntryImage::Pedestal);
        switch(_tr[i>>1]) {
        case D0:
          { unsigned x = frame.x;
            unsigned y = frame.y + frame.ny - 1;
            for(unsigned col=0; col<Pds::CsPad::ColumnsPerASIC; col++) {
              for (unsigned row=0; row < 2*Pds::CsPad::MaxRowsPerASIC; row++,off++)
                fprintf(fn, "%f ", double(*off) + (double(entry.content(x+col,y-row))-doff)/dn);
              fprintf(fn, "\n");
            }
            break; }
        case D90:
          { unsigned x = frame.x;
            unsigned y = frame.y;
            for(unsigned col=0; col<Pds::CsPad::ColumnsPerASIC; col++) {
              for (unsigned row=0; row < 2*Pds::CsPad::MaxRowsPerASIC; row++,off++)
                fprintf(fn, "%f ", double(*off) + (double(entry.content(x+row,y+col))-doff)/dn);
              fprintf(fn, "\n");
            }
            break; }
        case D180:
          { unsigned x = frame.x + frame.nx - 1;
            unsigned y = frame.y + frame.ny;
            for(unsigned col=0; col<Pds::CsPad::ColumnsPerASIC; col++) {
              for (unsigned row=0; row < 2*Pds::CsPad::MaxRowsPerASIC; row++,off++)
                fprintf(fn, "%f ", double(*off) + (double(entry.content(x-col,y+row))-doff)/dn);
              fprintf(fn, "\n");
            }
            break; }
        case D270:
          { unsigned x = frame.x + frame.nx - 1;
            unsigned y = frame.y + frame.ny - 1;
            for(unsigned col=0; col<Pds::CsPad::ColumnsPerASIC; col++) {
              for (unsigned row=0; row < 2*Pds::CsPad::MaxRowsPerASIC; row++,off++)
                fprintf(fn, "%f ", double(*off) + (double(entry.content(x-row,y-col))-doff)/dn);
              fprintf(fn, "\n");
            }
            break; }
        default:
          break;
        }
      }
      fclose(fn);
    
      // reconfigure
      msg = QString("Success.  Reconfigure to use new pedestals.");
    }

    QMessageBox::warning(this, 
                         "Write Pedestals", 
                         msg,
                         QMessageBox::Ok);
    if (fail && f)
      rename(nname.c_str(),oname.c_str());
    
    for(unsigned s=0; s<nframes; s++)
      delete _off[s];
    delete[] _off;
  }
}
