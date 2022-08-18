

function decodeUplink(input) {
  //initialize an object to store output data
  function byte_to_float(bytes){
    var bits = bytes[3]<<24 | bytes[2]<<16 | bytes[1]<<8 | bytes[0];
  // >>> is unsigned right shift. Zeros are shifted in at all times from the left
    var sign = (bits>>>31 === 0) ? 1.0 : -1.0;
    var e = bits>>>23 & 0xff;
  // Can code explicit leading 0 with 0 exponent. Otherwise, there is a
  // leading 1 implied.
  // Since we have constructed our significand/mantissa like we constructed
  // our integers, it is a factor of 2^23 too large (decimal point all the way)
  // right.
  // So, we correct that when we build the final number.
    var m = (e === 0) ? (bits & 0x7fffff)<<1 : (bits & 0x7fffff) | 0x800000;
    var f = sign * m * Math.pow(2, e - 127-23);
    return f;
  }
  var data = {}
  //create a field “myint16” for the data object
  //and then put together the 1st two 8-bit values in the input to make a 16-bit value
  //data.mystring = stringFromBytes(input.bytes.slice(0,4));
  //data.mynum = input.bytes[4] + (input.bytes[5]<<8) + (input.bytes[6]<<16) + (input.bytes[7]<<24);
  //data.last = input.bytes[8] + (input.bytes[9]<<8);
  //return the output data along with any errors or warnings
  data.name = 0
  data.top_weight = byte_to_float(input.bytes.slice(0,4));
  data.middle_weight = byte_to_float(input.bytes.slice(4,8));
  data.bottom_weight = byte_to_float(input.bytes.slice(8,12));
  data.battery_voltage = byte_to_float(input.bytes.slice(12,16));
  
  return {
    data: data,
    warnings: [],
    errors: []
  };
}
