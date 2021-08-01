/*
 * node.h
 */
#ifndef __NODE_H__
#define __NODE_H__

struct node {
  void *value;
  struct node *link;
};

#endif
