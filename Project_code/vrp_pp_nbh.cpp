#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include "vrp_pp_nbh.h"
#include <string>
#include <iostream>
#include "vrp_pp.h"

// Version 3.0 - 05.03.2021 - 08:30
// added: NBH::NBH_sort_by_fitness_increasing_feasfirst()

NBH_neighbor::NBH_neighbor(int _NBH_id, int _NBH_type, double _NBH_fitness, int _NBH_feasible, struct NBH_neighbor *_previous, struct NBH_neighbor *_next, class VRP_PP_PLAN *_ref)
{
	this->NBH_id = _NBH_id;
	this->NBH_type = _NBH_type;
	this->NBH_fitness = _NBH_fitness;
	this->NBH_feasible = _NBH_feasible;
	this->previous = _previous;
	this->next = _next;
	this->ref = _ref;
}

NBH_neighbor::~NBH_neighbor()
{
    this->previous = NULL;
	this->next = NULL;
	this->ref = NULL;
}

NBH::NBH(int _ID)
{
    // this constructor installs a new neighborhood that contains only the two dummy
    // members first and last

	this->NBH_id = _ID;
	this->NBH_members = 0;
	this->first = NULL;
	this->last = NULL;

}

void NBH::NBH_print_member_list(void)
{
    // this procedure prints a list of neighborhood members with id and fitness
	struct NBH_neighbor *Ptr;
    struct VRP_PP_EVALUATION EVAL;
    int pos=0;

	cout << "----- this is neighborhood "<<this->NBH_id<<" ("<<this->NBH_members<<" members) ----" << endl;
	Ptr = this->first;
	while (Ptr != NULL)
	{
		cout << "POS=" << pos << " ID="<<Ptr->NBH_id<<": fitness = "<<Ptr->NBH_fitness<< " REF at " << Ptr->ref << endl;
		Ptr = Ptr->next;
		pos++;
	}

    //struct VRP_PP_EVALUATION EVAL;
    cout << "- - - - - - - - - - - - - - -" << endl;
    if( this->NBH_members > 0)
    {
        cout << "bester index " << this->NBH_index_min_fitness() << endl;
        cout << "geringste Kosten " << this->NBH_get_average_fitness_from_to(this->NBH_index_min_fitness(), this->NBH_index_min_fitness()) << endl;
    }
    else
    {
        cout << "list is empty!" << endl;
    }
    cout << "-----------------------------" << endl;

}

NBH::~NBH()
{
    // this is the deconstructor
    if( this->first != NULL)
    {
        cout << "mergency stop" << endl;
        exit(0);
    }

    if( this->last != NULL)
    {
        cout << "mergency stop" << endl;
        exit(0);
    }
}

void NBH::NBH_add_plan(int Member_ID, class VRP_PP_PLAN *_PLAN)
{
    // this procedure appends the plan *_PLAN to the collection of already contained
    // neighborhood members

    // create new neighborhood list element
	struct NBH_neighbor *AdditionalNBHElem;
	AdditionalNBHElem = new struct NBH_neighbor(this->NBH_members, 0, -1, -1, NULL, NULL, _PLAN);

    // append to existing member list
	if (this->first != NULL)
	{

	    AdditionalNBHElem->next = NULL;
	    AdditionalNBHElem->previous = this->last;
	    this->last->next = AdditionalNBHElem;
	    this->last = AdditionalNBHElem;
	}
	else
	{
		this->first = AdditionalNBHElem;
		this->last = AdditionalNBHElem;
	}

    // increase member counter
	this->NBH_members++;
}

class VRP_PP_PLAN *NBH::NBH_get_pointer_to_plan_at_pos(int _POS)
{
    // this procedure returns the reference (pointer) to the element that is
    // at position _POS in the liste of neighborhod elements

	class VRP_PP_PLAN *ret_val;
	ret_val = nullptr;
	int i;

	if (_POS < this->NBH_members)
	{
		struct NBH_neighbor *Ptr;
		Ptr = this->first;
		for (i = 0; i < _POS; i++)
        {
           Ptr = Ptr->next;
        }

		ret_val = Ptr->ref;
	}

	return(ret_val);
}

void NBH::NBH_update_fitness_at_pos(double _NewFitness, int _Feasible, int _POS)

{
	// this procedure updates the fitness value of the element that is
	// at position _POS in the liste of neighborhod elements

	if (_POS < this->NBH_members)
	{
		int i;
		struct NBH_neighbor *Ptr;
		Ptr = this->first;
		for (i = 0; i < _POS; i++)
			Ptr = Ptr->next;
		Ptr->NBH_fitness = _NewFitness;
		Ptr->NBH_feasible = _Feasible;
	}

}

void NBH::NBH_remove_plan(int Member_ID)
{
	// this procedure deletes the neighborhood member with Member_ID

	if (this->NBH_members > 0)
	{
		struct NBH_neighbor *PtrToDelete;

		PtrToDelete = this->first;
		while ( (PtrToDelete->NBH_id != Member_ID) && (PtrToDelete != NULL) )
		{
			PtrToDelete = PtrToDelete->next;
		}
		if (PtrToDelete != NULL)
		{
          // element to be delete has been found
			if (PtrToDelete == this->first)
			{
				if (this->NBH_members == 1)
				{
					this->first = NULL;
					this->last = NULL;
				}
				else
				{
					this->first = this->first->next;
					this->first->previous = NULL;
				}
				if (PtrToDelete->ref != NULL)
				{
				    PtrToDelete->ref->clear();
					PtrToDelete->ref->~VRP_PP_PLAN();
					delete (PtrToDelete->ref);
				}
				delete PtrToDelete;
				this->NBH_members--;
			}
			else
			    if (PtrToDelete == this->last)
			    {
					if (this->NBH_members == 1)
					{
						this->first = NULL;
						this->last = NULL;
					}
					else
					{
						this->last = this->last->previous;
						this->last->next = NULL;
					}
					if (PtrToDelete->ref != NULL)
					{
						PtrToDelete->ref->clear();
                        PtrToDelete->ref->~VRP_PP_PLAN();
						delete (PtrToDelete->ref);
					}
					delete PtrToDelete;
					this->NBH_members--;
			    }
			    else
			    {
					PtrToDelete->previous->next = PtrToDelete->next;
					PtrToDelete->next->previous = PtrToDelete->previous;

					PtrToDelete->previous = NULL;
					PtrToDelete->next = NULL;

					if (PtrToDelete->ref != NULL)
					{
						PtrToDelete->ref->clear();
                        PtrToDelete->ref->~VRP_PP_PLAN();
						delete (PtrToDelete->ref);
					}
					delete PtrToDelete;
					this->NBH_members--;
			    }
		}
	}
}

void NBH::NBH_remove_plan_at_pos(int _POSITION)
{
	// this procedure deletes the neighborhood member with Member_ID

	//cout << "HALLO" << endl;
	if (this->NBH_members > 0)
	{
		int Current_Position = 0;
		struct NBH_neighbor *PtrToDelete;

		PtrToDelete = this->first;
		while ((Current_Position != _POSITION) && (PtrToDelete != NULL))
		{
			Current_Position++;
			PtrToDelete = PtrToDelete->next;
		}
		if (PtrToDelete != NULL)
		{
			// element to be delete has been found
			if (PtrToDelete == this->first)
			{
				if (this->NBH_members == 1)
				{
					this->first = NULL;
					this->last = NULL;
				}
				else
				{
					this->first = this->first->next;
					this->first->previous = NULL;
				}
				if (PtrToDelete->ref != NULL)
				{
				    //cout << "AA+" << endl;
					PtrToDelete->ref->clear();
					PtrToDelete->ref->~VRP_PP_PLAN();
					delete PtrToDelete->ref;

					//cout << "AA-" << endl;
				}
				//cout << "A+" << endl;

				delete PtrToDelete;
				//cout << "A-" << endl;
				this->NBH_members--;
			}
			else
			if (PtrToDelete == this->last)
			{
				if (this->NBH_members == 1)
				{
					this->first = NULL;
					this->last = NULL;
				}
				else
				{
					this->last = this->last->previous;
					this->last->next = NULL;
				}
				if (PtrToDelete->ref != NULL)
				{
				    //cout << "BB+" << endl;
					PtrToDelete->ref->clear();
					PtrToDelete->ref->~VRP_PP_PLAN();
					delete PtrToDelete->ref;
					//cout << "BB-" << endl;
				}
				//cout << "B+" << endl;
				delete PtrToDelete;
				//cout << "B-" << endl;
				this->NBH_members--;
			}
			else
			{
				PtrToDelete->previous->next = PtrToDelete->next;
				PtrToDelete->next->previous = PtrToDelete->previous;

				PtrToDelete->previous = NULL;
				PtrToDelete->next = NULL;

				if (PtrToDelete->ref != NULL)
				{
				    //cout << "CC+" << endl;
					PtrToDelete->ref->clear();
					delete PtrToDelete->ref;
					//cout << "CC-" << endl;
				}
				//cout << "C+" << endl;
				delete PtrToDelete;
				//cout << "C-" << endl;
				this->NBH_members--;
			}
		}
	}
}

/*
void NBH::NBH_reset(class VRP_PP_PROBLEM *prob)
{
    // this procedure removes all neighborhood members and re-initializes all attributes
	while (this->first != NULL)
	{
	    //cout << "removed " << this->first->NBH_id << endl;
	    this->NBH_remove_plan_at_pos(0);

	}
	//this->NBH_id = 17;
}
*/

void NBH::NBH_reset(class VRP_PP_PROBLEM *prob)
{
    class NBH_neighbor *PtrToNBHElem;
    PtrToNBHElem = this->first;

    /*
    while ( PtrToNBHElem != NULL )
    {
        cout << "NBH-Elem" << PtrToNBHElem->NBH_id << " with plan at " << PtrToNBHElem->ref << endl;
        PtrToNBHElem = PtrToNBHElem->next;
    }
    */

    PtrToNBHElem = this->first;
    while( PtrToNBHElem != NULL)
    {
        // remove plan referred by NBHelem
        PtrToNBHElem->ref->~VRP_PP_PLAN();
        delete PtrToNBHElem->ref;
        PtrToNBHElem->ref = NULL;
        PtrToNBHElem = PtrToNBHElem->next;
    }

    PtrToNBHElem = this->first;

    /*
    while ( PtrToNBHElem != NULL )
    {
        cout << "NBH-Elem" << PtrToNBHElem->NBH_id << " with plan at " << PtrToNBHElem->ref << endl;
        PtrToNBHElem = PtrToNBHElem->next;
    }
    */

    PtrToNBHElem = this->first;

    while( PtrToNBHElem != NULL)
    {
        this->NBH_remove_plan(PtrToNBHElem->NBH_id);
        PtrToNBHElem = this->first;

    }
	//cout << "print after emptying" << endl;
    //this->NBH_print_member_list();

	//this->NBH_id = 17;
	//exit(0);
}
int NBH::NBH_index_min_fitness()
{
	int ret_val = 0, i=0;
	double best_fit = 100000000;

	struct NBH_neighbor *Ptr;
	Ptr = this->first;
	while (Ptr != NULL)
	{
		if (Ptr->NBH_fitness < best_fit)
		{
			best_fit = Ptr->NBH_fitness;
			ret_val = i;
		}
		i++;
		Ptr = Ptr->next;
	}
	return(ret_val);
}

void NBH::NBH_sort_by_fitness_increasing()
{
	if (this->NBH_members > 1)
	{
		struct NBH_neighbor *Ptr;

		int i;
		for (i = 0; i < this->NBH_members - 1; i++)
		{
		    Ptr = this->first;
		    while (Ptr->next != NULL)
		    {
				if (Ptr->NBH_fitness > Ptr->next->NBH_fitness)
			    {
					//printf("Tausch notwendig\n");
					this->NBH_swap_with_next(Ptr);

			    }
				else
				{
					// printf("kein Tausch notwendig\n");
					Ptr = Ptr->next;
				}

		    }
		}
	}
}

void NBH::NBH_sort_by_fitness_increasing_feasfirst()
{
    // Version 3.0 - 05.03.2021 - 08:30

	if (this->NBH_members > 1)
	{
		struct NBH_neighbor *Ptr;

		int i;

		// first, sort by increasing fitness values
		for (i = 0; i < this->NBH_members - 1; i++)
		{
		    Ptr = this->first;
		    while (Ptr->next != NULL)
		    {
				if (Ptr->NBH_fitness > Ptr->next->NBH_fitness)
			    {
					//printf("Tausch notwendig\n");
					this->NBH_swap_with_next(Ptr);

			    }
				else
				{
					// printf("kein Tausch notwendig\n");
					Ptr = Ptr->next;
				}

		    }
		}

		// second, move feasible prior to infeasible elements
		for (i = 0; i < this->NBH_members - 1; i++)
		{
		    Ptr = this->first;
		    while (Ptr->next != NULL)
		    {
				if (Ptr->NBH_feasible < Ptr->next->NBH_feasible)
			    {
					//printf("Tausch notwendig\n");
					this->NBH_swap_with_next(Ptr);

			    }
				else
				{
					// printf("kein Tausch notwendig\n");
					Ptr = Ptr->next;
				}

		    }
		}
	}
}

void NBH::NBH_swap_with_next(struct NBH_neighbor *Ptr)
{
	if (this->NBH_members < 2)
	{
        // nothing to swap
	}
	else
	{
		if (this->NBH_members == 2)
		{
            // list consists of two members
			class NBH_neighbor *TmpPtr;
			TmpPtr = this->last;
			this->last = this->first;
			this->first = TmpPtr;

			this->first->previous = NULL;
			this->first->next = this->last;
			this->last->previous = this->first;
			this->last->next = NULL;
		}
		else
		{
            // list consists of more than two members
			if (Ptr == this->first)
			{
                // vertauschen des ersten und zweiten elements der liste
				//class NBH_neighbor *TmpPtr;
				Ptr->next->previous = NULL;
				this->first = Ptr->next;
				Ptr->next = this->first->next;
				Ptr->previous = this->first;
				this->first->next->previous = Ptr;
				first->next = Ptr;
			}
			else
			{
				if (Ptr->next == this->last)
				{
                    // vertauschen der letzten beiden elemente der liste
					Ptr->previous->next = this->last;
					this->last->previous = Ptr->previous;
					Ptr->next = NULL;
					Ptr->previous = this->last;
					this->last->next = Ptr;
					this->last = Ptr;
				}
				else
				{
                    // vertauschen von zwei mittelelementen der Liste
					class NBH_neighbor *Ptr2;

					Ptr2 = Ptr->next;

                    Ptr->previous->next = Ptr->next;
					Ptr->next->previous = Ptr->previous;

                    Ptr->previous = NULL;
					Ptr->next = NULL;

					Ptr->previous = Ptr2;
					Ptr->next = Ptr2->next;

					Ptr2->next->previous = Ptr;
					Ptr2->next = Ptr;
				}
			}
		}
	}

}

double NBH::NBH_get_average_fitness_from_to(int _StartPos, int _EndPos)
{
	double ret_val = 0.0;
	int i;

	class NBH_neighbor *Ptr;

	Ptr = this->first;
	for (i = 0; i < _StartPos; i++ )
	{
		Ptr = Ptr->next;
	}
	for (i = _StartPos; i <= _EndPos; i++)
	{
		ret_val = ret_val + Ptr->NBH_fitness;
		Ptr = Ptr->next;
	}

	ret_val = ret_val / (double)(_EndPos + 1 - _StartPos);

	return(ret_val);
}
