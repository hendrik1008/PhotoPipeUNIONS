#include <stdio.h>
#include <stdlib.h>

#include "queue_globals.h"
#include "queue_types.h"
#include "utils_globals.h"
#include "utils_macros.h"

queue_struct	*front, *rear;

void createQ()
{
/****** createQ ***************************************************
 *
 *  NAME	
 *      createQ - Create a queue structure
 *
 *  SYNOPSIS
 *      void createQ()
 *
 *  FUNCTION
 *      This funtion initializes the queueing system
 *
 *  INPUTS
 *      None
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      None
 *
 *  MODIFIED GLOBALS
 *      Sets the pointers front and rear to NULL
 *
 *  NOTES
 *      This routine initializes the queue structure. After a call to
 *      it, a user can savely use all the other routines in the package.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd 05-07-1996
 *
 ******************************************************************************/
    front = rear = NULL;
}

void putQ(int i)
/****** putQ() ***************************************************
 *
 *  NAME	
 *      putQ - Add an element to the queue
 *
 *  SYNOPSIS
 *      void putQ(int)
 *
 *  FUNCTION
 *      Add an element to the end of the queue and save the info into it
 *
 *  INPUTS
 *      i - An integer denoting the queued information
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      None
 *
 *  MODIFIED GLOBALS
 *      Adds a queue element on the heap and corrects the fornt and rear
 *      ponters
 *
 *  NOTES
 *      This routine allows one to add an identifier to the queue.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd  05-07-1996
 *
 ******************************************************************************/
{
    queue_struct *t_q;

    ED_CALLOC(t_q, "putQ", queue_struct, 1);
    t_q->id = i;
    if (rear == NULL) {
       front = t_q;
    } else {
       rear->next = t_q;
    }
    rear = t_q;
}

void getQ(int *i)
/****** getQ ***************************************************
 *
 *  NAME	
 *      getQ - Get the identifier off the head of the queue
 *
 *  SYNOPSIS
 *      void getQ(int *)
 *
 *  FUNCTION
 *      Get the information off the head of the queue
 *
 *  INPUTS
 *      None
 *
 *  RESULTS
 *      i   - The identifier from the head of the queue
 *
 *  RETURNS
 *      None
 *
 *  MODIFIED GLOBALS
 *      Removes the heap queue element from the heap and corrects the
 *      front and rear pointers.
 *
 *  NOTES
 *      This routine provides the caller with the information queued at
 *      the head of the queue.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd  05-07-1996
 *
 ******************************************************************************/
{
    queue_struct *t_q;

    t_q = front;
    *i = front->id;
    front = front->next;
    if (front == NULL) rear = NULL;
    ED_FREE(t_q);
}

int QisMT()
/****** QisMT ***************************************************
 *
 *  NAME	
 *      QisMT - Check if the queue is empty
 *
 *  SYNOPSIS
 *      int QisMT()
 *
 *  FUNCTION
 *      Checks is there are no queue elements in the queue
 *
 *  INPUTS
 *      None
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      TRUE  - If the queue is an empty queue
 *      FALSE - If there are still 1 or more elements left in the queue
 *
 *  MODIFIED GLOBALS
 *      None
 *
 *  NOTES
 *      This routine provides the user with a means of finding out if
 *      there are still elements on the queue.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd  05-07-1996
 *
 ******************************************************************************/
{
    return ((front == NULL) && (rear == NULL));
}

int firstQ()
/****** firstQ() ***************************************************
 *
 *  NAME	
 *      firstQ - Get the identifier from the head of the queue
 *
 *  SYNOPSIS
 *      int firstQ()
 *
 *  FUNCTION
 *      Get theinformation from the head of the queue leaving it in tact
 *
 *  INPUTS
 *      None
 *
 *  RESULTS
 *      None
 *
 *  RETURNS
 *      i - The identifier stored at the head of the queue
 *
 *  MODIFIED GLOBALS
 *      None
 *
 *  NOTES
 *      This routine provides the user with information stored at the
 *      head of the queue, without actually removing the head queue
 *      element.
 *
 *  BUGS
 *
 *  AUTHOR
 *      E.R. Deul  - dd  05-07-1996
 *
 ******************************************************************************/
{
    return (front->id);
}

