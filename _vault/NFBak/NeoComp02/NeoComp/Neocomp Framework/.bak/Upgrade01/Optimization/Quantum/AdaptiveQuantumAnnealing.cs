﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Diagnostics.Contracts;
using NeoComp.Core;

namespace NeoComp.Optimization.Quantum
{
    [ContractClass(typeof(AdaptiveQuantumAnnealingContract))]
    public abstract class AdaptiveQuantumAnnealing : IAnnealing
    {
        protected AdaptiveQuantumAnnealing(IEnumerable<IQuantumStatedItem> items)
        {
            Contract.Requires(items != null);

            this.items = items.ToArray();

            if (this.items.Length == 0) throw new ArgumentException("Item collection is empty.", "items");

            this.prevStates = this.items.Select(i => i.State).ToArray();
        }

        IQuantumStatedItem[] items;

        QuantumState[] prevStates;

        double? currentEnergy;

        public int SolutionVectorDimension
        {
            get { return items.Length; }
        }

        public double CurrentEnergy
        {
            get { return currentEnergy ?? 1.0; }
        }

        protected abstract double GetTunnelingFieldStrength(double currentEnergy);

        protected abstract double GetWrongSolutionAcceptTreshold(double currentEnergy, double newEnergy);

        protected abstract double ComputeEnergy();

        protected abstract void Improved();

        protected abstract void Rejected();

        public virtual void Reset()
        {
            currentEnergy = null;
        }

        public bool Step()
        {
            if (!currentEnergy.HasValue)
            {
                InitItems();
                currentEnergy = ComputeEnergy();
                Save();
            }

            Tunneling();

            double newEnergy = ComputeEnergy();

            if ((newEnergy < currentEnergy.Value) ||
                (newEnergy == currentEnergy.Value && RandomGenerator.FiftyPercentChance) ||
                (RandomGenerator.Random.NextDouble() < GetWrongSolutionAcceptTreshold(currentEnergy.Value, newEnergy)))
            {
                currentEnergy = newEnergy;
                Save();
                Improved();
                return true;
            }
            else
            {
                Restore();
                Rejected();
                return false;
            }
        }

        internal void Blend(AdaptiveQuantumAnnealing other)
        {
            int count = Math.Min(items.Length, other.items.Length);
            for (int idx = 0; idx < count; idx++)
            {
                items[idx].State.Blend(other.items[idx].State);
            }
            Save();
        }

        private void InitItems()
        {
            for (int idx = 0; idx < items.Length; idx++) items[idx].State = GetItemInitState(idx);
        }

        protected virtual QuantumState GetItemInitState(int itemIndex)
        {
            return QuantumState.Random();
        }

        private void Tunneling()
        {
            double strength = GetTunnelingFieldStrength(currentEnergy.Value);
            double s2 = strength / 2.0;
            foreach (var item in items)
            {
                double noise = (RandomGenerator.Random.NextDouble() * strength) - s2;
                item.State += noise;
            }
        }

        private void Save()
        {
            for (int idx = 0; idx < items.Length; idx++) prevStates[idx] = items[idx].State;
        }

        private void Restore()
        {
            for (int idx = 0; idx < items.Length; idx++) items[idx].State = prevStates[idx];
        }
    }

    [ContractClassFor(typeof(AdaptiveQuantumAnnealing))]
    abstract class AdaptiveQuantumAnnealingContract : AdaptiveQuantumAnnealing
    {
        protected AdaptiveQuantumAnnealingContract()
            : base(null)
        {
        }
        
        protected override double ComputeEnergy()
        {
            Contract.Ensures(Contract.Result<double>() >= 0.0 && Contract.Result<double>() <= 1.0);
            return 0;
        }

        protected override double GetTunnelingFieldStrength(double currentEnergy)
        {
            Contract.Requires(currentEnergy >= 0.0 && currentEnergy <= 1.0);
            Contract.Ensures(Contract.Result<double>() >= 0.0 && Contract.Result<double>() <= 1.0);
            return 0;
        }

        protected override double GetWrongSolutionAcceptTreshold(double currentEnergy, double newEnergy)
        {
            Contract.Requires(currentEnergy >= 0.0 && currentEnergy <= 1.0);
            Contract.Requires(newEnergy >= 0.0 && newEnergy <= 1.0);
            Contract.Ensures(Contract.Result<double>() >= 0.0 && Contract.Result<double>() <= 1.0);
            return 0;
        }
    }
}
