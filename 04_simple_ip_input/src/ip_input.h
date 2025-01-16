#ifndef _IP_INPUT_H_
#define _IP_INPUT_H_


/**
 *  \brief
 *      Get IP address from user. Left and Right will select which
 *      byte will be set. Up and Down will increase/decrease the value
 *      of the selected byte. Holding down C while pressing Up or Down
 *      will change the value by 10. 
 *  
 *      I'm assuming the input will be correctly formatted.
 *
 *  \param ipaddr 
 *     Char array with IP address with leading zeros. ex: "192.168.001.002"
 *     This will be updated if the user presses A. B will cancel without 
 *     updating the ipaddr character array.
 */

void getIPFromUser( char* ipaddr );


#endif // _IP_INPUT_H_
