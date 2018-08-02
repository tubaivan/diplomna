#ifndef __SERIAL_PORT_H_INCLUDED__
#define __SERIAL_PORT_H_INCLUDED__
// written by Yavor Vutov
#include <boost/bind.hpp> 
#include <boost/asio.hpp> 
#include <boost/asio/serial_port.hpp> 
#include <boost/function.hpp>
#include <vector>
#include <deque>
#include <string>
#include "utils.h"

class SerialPort
{
protected:
  static const int BUFFER_SIZE = 2000;
  boost::asio::io_service& io_service; 
  boost::asio::serial_port serial_port; 
  unsigned char read_msg[BUFFER_SIZE]; // read buffer
  std::deque<std::vector<unsigned char> > write_msgs; // buffered write data
  std::vector<unsigned char> current_write;
  typedef void consume_callback_type(const unsigned char* buffer, size_t count);
  typedef boost::function<consume_callback_type> consume_function;
  consume_function consume;
  typedef void error_callback_type(const boost::system::error_code& error);
  typedef boost::function<error_callback_type> error_callback;
  error_callback error;
  bool to_stop_reading;
  bool reading;
  std::string name;
  unsigned shrink_counter;
public:
  const std::string& GetName() {return name;}
  void SetBaudRate(int baud);
  void SetBits(int bits);
  SerialPort(boost::asio::io_service& io_service) ;
  virtual ~SerialPort();
  void LooseControllyngTTY();
  // wee keep the boost prototypes
  boost::asio::io_service& get_io_service();
  void assign(int a);

  //callback setters
  void SetConsumer(consume_function consum);
  void SetErrorHandler(error_callback er);

  void Open(const std::string& device, unsigned int baud = 9600); //opens port
  int GetCharacterSize(){
    boost::asio::serial_port_base::character_size csize;
    serial_port.get_option(csize);
    return csize.value();
  }
  void SetCharacterSize(int size)
  {
    boost::asio::serial_port_base::character_size csize(size);
    serial_port.set_option(csize);
  }    
  void SetRaw(); // set raw port settings
  void Drain(); //drains all curent input

  void FlushInput();
  void FlushOutput();
  void FlushBoth();
  // calls consumer callback
  void CallConsume(const unsigned char* msg, size_t bytes_transfered);

  // calls error callback
  void HandleError(const boost::system::error_code& error_code);

  //async write
  void Write(std::vector<unsigned char> &&msg); // pass the write data to the do_write function via the io service in the other thread
  template<class Iter>
  void Write(Iter start, Iter end) // pass the write data to the do_write function via the io service in the other thread
  {
    Write(std::move(std::vector<unsigned char>(start, end)));
  }

  // blocking write
  size_t BlockingWrite(const std::vector<unsigned char> msg); // pass the write data to the do_write function via the io service in the other thread 
  
  void Close(); // call the do_close function via the io service in the other thread

  bool IsStoppingReading()const{return to_stop_reading;}


    
  
  bool IsReading()const {return reading;}
  // start reading
  void ReadStart(void); // Start an asynchronous read and call read_complete when it completes or fails 

  // stop reading & writing
  void Stop(void);
  void StopReadingAftherRecieve(void)
  {
    to_stop_reading=true;
  }

  
  // blocking close in current threads
  void DoClose();
  size_t write_queue_size()const
  {
    return write_msgs.size();
  }
private: 
  static const int max_read_length = 512; // maximum amount of data to read in one operation 
        
  void ReadComplete(const boost::system::error_code& error, size_t bytes_transferred) ;
  void DoWrite(std::vector<unsigned char> &&msg);
  void WriteStart(void);
  void WriteComplte(const boost::system::error_code& error);
  // close if not canceled (on error)
  void DoCancelOrClose(const boost::system::error_code& error);
}; 
#endif