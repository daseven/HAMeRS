#include "flow/flow_models/four-eqn_conservative/FlowModelFourEqnConservative.hpp"

#include "flow/flow_models/four-eqn_conservative/FlowModelBoundaryUtilitiesFourEqnConservative.hpp"

FlowModelFourEqnConservative::FlowModelFourEqnConservative(
    const std::string& object_name,
    const tbox::Dimension& dim,
    const boost::shared_ptr<geom::CartesianGridGeometry>& grid_geometry,
    const hier::IntVector& num_ghosts,
    const int& num_species,
    const boost::shared_ptr<tbox::Database>& flow_model_db):
        FlowModel(
            object_name,
            dim,
            grid_geometry,
            num_ghosts,
            num_species,
            num_species + dim.getValue() + 1,
            flow_model_db),
        d_num_subghosts_density(-hier::IntVector::getOne(d_dim)),
        d_num_subghosts_mass_fraction(-hier::IntVector::getOne(d_dim)),
        d_num_subghosts_velocity(-hier::IntVector::getOne(d_dim)),
        d_num_subghosts_internal_energy(-hier::IntVector::getOne(d_dim)),
        d_num_subghosts_pressure(-hier::IntVector::getOne(d_dim)),
        d_num_subghosts_sound_speed(-hier::IntVector::getOne(d_dim)),
        d_num_subghosts_temperature(-hier::IntVector::getOne(d_dim)),
        d_num_subghosts_dilatation(-hier::IntVector::getOne(d_dim)),
        d_num_subghosts_vorticity(-hier::IntVector::getOne(d_dim)),
        d_num_subghosts_enstrophy(-hier::IntVector::getOne(d_dim)),
        d_num_subghosts_convective_flux_x(-hier::IntVector::getOne(d_dim)),
        d_num_subghosts_convective_flux_y(-hier::IntVector::getOne(d_dim)),
        d_num_subghosts_convective_flux_z(-hier::IntVector::getOne(d_dim)),
        d_num_subghosts_max_wave_speed_x(-hier::IntVector::getOne(d_dim)),
        d_num_subghosts_max_wave_speed_y(-hier::IntVector::getOne(d_dim)),
        d_num_subghosts_max_wave_speed_z(-hier::IntVector::getOne(d_dim)),
        d_num_subghosts_diffusivities(-hier::IntVector::getOne(d_dim)),
        d_subghost_box_density(hier::Box::getEmptyBox(dim)),
        d_subghost_box_mass_fraction(hier::Box::getEmptyBox(dim)),
        d_subghost_box_velocity(hier::Box::getEmptyBox(dim)),
        d_subghost_box_internal_energy(hier::Box::getEmptyBox(dim)),
        d_subghost_box_pressure(hier::Box::getEmptyBox(dim)),
        d_subghost_box_sound_speed(hier::Box::getEmptyBox(dim)),
        d_subghost_box_temperature(hier::Box::getEmptyBox(dim)),
        d_subghost_box_dilatation(hier::Box::getEmptyBox(dim)),
        d_subghost_box_vorticity(hier::Box::getEmptyBox(dim)),
        d_subghost_box_enstrophy(hier::Box::getEmptyBox(dim)),
        d_subghost_box_convective_flux_x(hier::Box::getEmptyBox(dim)),
        d_subghost_box_convective_flux_y(hier::Box::getEmptyBox(dim)),
        d_subghost_box_convective_flux_z(hier::Box::getEmptyBox(dim)),
        d_subghost_box_max_wave_speed_x(hier::Box::getEmptyBox(dim)),
        d_subghost_box_max_wave_speed_y(hier::Box::getEmptyBox(dim)),
        d_subghost_box_max_wave_speed_z(hier::Box::getEmptyBox(dim)),
        d_subghost_box_diffusivities(hier::Box::getEmptyBox(dim)),
        d_subghostcell_dims_density(hier::IntVector::getZero(d_dim)),
        d_subghostcell_dims_mass_fraction(hier::IntVector::getZero(d_dim)),
        d_subghostcell_dims_velocity(hier::IntVector::getZero(d_dim)),
        d_subghostcell_dims_internal_energy(hier::IntVector::getZero(d_dim)),
        d_subghostcell_dims_pressure(hier::IntVector::getZero(d_dim)),
        d_subghostcell_dims_sound_speed(hier::IntVector::getZero(d_dim)),
        d_subghostcell_dims_temperature(hier::IntVector::getZero(d_dim)),
        d_subghostcell_dims_dilatation(hier::IntVector::getZero(d_dim)),
        d_subghostcell_dims_vorticity(hier::IntVector::getZero(d_dim)),
        d_subghostcell_dims_enstrophy(hier::IntVector::getZero(d_dim)),
        d_subghostcell_dims_convective_flux_x(hier::IntVector::getZero(d_dim)),
        d_subghostcell_dims_convective_flux_y(hier::IntVector::getZero(d_dim)),
        d_subghostcell_dims_convective_flux_z(hier::IntVector::getZero(d_dim)),
        d_subghostcell_dims_max_wave_speed_x(hier::IntVector::getZero(d_dim)),
        d_subghostcell_dims_max_wave_speed_y(hier::IntVector::getZero(d_dim)),
        d_subghostcell_dims_max_wave_speed_z(hier::IntVector::getZero(d_dim)),
        d_subghostcell_dims_diffusivities(hier::IntVector::getZero(d_dim))
{
    d_eqn_form.reserve(d_num_eqn);
    
    // Set the equation forms for partial density.
    for (int si = 0; si < d_num_species; si++)
    {
        d_eqn_form.push_back(EQN_FORM::CONSERVATIVE);
    }
    
    // Set the equation forms for momentum.
    for (int di = 0; di < d_dim.getValue(); di++)
    {
        d_eqn_form.push_back(EQN_FORM::CONSERVATIVE);
    }
    
    // Set the equation form for total energy.
    d_eqn_form.push_back(EQN_FORM::CONSERVATIVE);
    
    // Set the bounds for the variables.
    d_Y_bound_lo = -0.001;
    d_Y_bound_up = 1.001;
    
    /*
     * Initialize the conservative variables.
     */
    
    d_variable_partial_density = boost::shared_ptr<pdat::CellVariable<double> > (
        new pdat::CellVariable<double>(d_dim, "partial density", d_num_species));
    
    d_variable_momentum = boost::shared_ptr<pdat::CellVariable<double> > (
        new pdat::CellVariable<double>(d_dim, "momentum", d_dim.getValue()));
    
    d_variable_total_energy = boost::shared_ptr<pdat::CellVariable<double> > (
        new pdat::CellVariable<double>(d_dim, "total energy", 1));
    
    /*
     * Initialize d_equation_of_state_mixing_rules_manager and get the equation of state mixing rules object.
     */
    
    if (flow_model_db->keyExists("equation_of_state"))
    {
        d_equation_of_state_str = flow_model_db->getString("equation_of_state");
    }
    else if (flow_model_db->keyExists("d_equation_of_state_str"))
    {
        d_equation_of_state_str = flow_model_db->getString("d_equation_of_state_str");
    }
    else
    {
        TBOX_ERROR(d_object_name
            << ": "
            << "No key 'equation_of_state'/'d_equation_of_state_str' found in data for flow model"
            << std::endl);
    }
    
    boost::shared_ptr<tbox::Database> equation_of_state_mixing_rules_db;
    
    if (flow_model_db->keyExists("Equation_of_state_mixing_rules"))
    {
        equation_of_state_mixing_rules_db = flow_model_db->getDatabase("Equation_of_state_mixing_rules");
    }
    else if (flow_model_db->keyExists("d_equation_of_state_mixing_rules_db"))
    {
        equation_of_state_mixing_rules_db = flow_model_db->getDatabase("d_equation_of_state_mixing_rules_db");
    }
    else
    {
        TBOX_ERROR(d_object_name
            << ": "
            << "No key 'Equation_of_state_mixing_rules'/'d_equation_of_state_mixing_rules_db' found in data for flow model"
            << std::endl);
    }
    
    d_equation_of_state_mixing_rules_manager.reset(new EquationOfStateMixingRulesManager(
        "d_equation_of_state_mixing_rules_manager",
        d_dim,
        d_num_species,
        MIXING_CLOSURE_MODEL::ISOTHERMAL_AND_ISOBARIC,
        equation_of_state_mixing_rules_db,
        d_equation_of_state_str));
    
    d_equation_of_state_mixing_rules =
        d_equation_of_state_mixing_rules_manager->getEquationOfStateMixingRules();
    
    /*
     * Initialize d_equation_of_mass_diffusivity_mixing_rules and get the equation of
     * mass diffusivity mixing rules object.
     */
    
    if ((flow_model_db->keyExists("equation_of_mass_diffusivity")) ||
        (flow_model_db->keyExists("d_equation_of_mass_diffusivity_str")))
    {
        if (flow_model_db->keyExists("equation_of_mass_diffusivity"))
        {
            d_equation_of_mass_diffusivity_str =
                flow_model_db->getString("equation_of_mass_diffusivity");
        }
        else if (flow_model_db->keyExists("d_equation_of_mass_diffusivity_str"))
        {
            d_equation_of_mass_diffusivity_str =
                flow_model_db->getString("d_equation_of_mass_diffusivity_str");
        }
        
        boost::shared_ptr<tbox::Database> equation_of_mass_diffusivity_mixing_rules_db;
        
        if (flow_model_db->keyExists("Equation_of_mass_diffusivity_mixing_rules"))
        {
            equation_of_mass_diffusivity_mixing_rules_db =
                flow_model_db->getDatabase("Equation_of_mass_diffusivity_mixing_rules");
        }
        else if (flow_model_db->keyExists("d_equation_of_mass_diffusivity_mixing_rules_db"))
        {
            equation_of_mass_diffusivity_mixing_rules_db =
                flow_model_db->getDatabase("d_equation_of_mass_diffusivity_mixing_rules_db");
        }
        else
        {
            TBOX_ERROR(d_object_name
                << ": "
                << "No key 'Equation_of_mass_diffusivity_mixing_rules'/"
                << "'d_equation_of_mass_diffusivity_mixing_rules_db' found in data for flow model"
                << std::endl);
        }
        
        d_equation_of_mass_diffusivity_mixing_rules_manager.reset(
            new EquationOfMassDiffusivityMixingRulesManager(
                "d_equation_of_mass_diffusivity_mixing_rules_manager",
                d_dim,
                d_num_species,
                MIXING_CLOSURE_MODEL::ISOTHERMAL_AND_ISOBARIC,
                equation_of_mass_diffusivity_mixing_rules_db,
                d_equation_of_mass_diffusivity_str));
        
        d_equation_of_mass_diffusivity_mixing_rules =
            d_equation_of_mass_diffusivity_mixing_rules_manager->
                getEquationOfMassDiffusivityMixingRules();
    }
    
    /*
     * Initialize d_equation_of_shear_viscosity_mixing_rules_manager and get the equation of
     * shear viscosity mixing rules object.
     */
    
    if ((flow_model_db->keyExists("equation_of_shear_viscosity")) ||
        (flow_model_db->keyExists("d_equation_of_shear_viscosity_str")))
    {
        if (flow_model_db->keyExists("equation_of_shear_viscosity"))
        {
            d_equation_of_shear_viscosity_str =
                flow_model_db->getString("equation_of_shear_viscosity");
        }
        else if (flow_model_db->keyExists("d_equation_of_shear_viscosity_str"))
        {
            d_equation_of_shear_viscosity_str =
                flow_model_db->getString("d_equation_of_shear_viscosity_str");
        }
        
        boost::shared_ptr<tbox::Database> equation_of_shear_viscosity_mixing_rules_db;
        
        if (flow_model_db->keyExists("Equation_of_shear_viscosity_mixing_rules"))
        {
            equation_of_shear_viscosity_mixing_rules_db =
                flow_model_db->getDatabase("Equation_of_shear_viscosity_mixing_rules");
        }
        else if (flow_model_db->keyExists("d_equation_of_shear_viscosity_mixing_rules_db"))
        {
            equation_of_shear_viscosity_mixing_rules_db =
                flow_model_db->getDatabase("d_equation_of_shear_viscosity_mixing_rules_db");
        }
        else
        {
            TBOX_ERROR(d_object_name
                << ": "
                << "No key 'Equation_of_shear_viscosity_mixing_rules'/"
                << "'d_equation_of_shear_viscosity_mixing_rules_db' found in data for flow model"
                << std::endl);
        }
        
        d_equation_of_shear_viscosity_mixing_rules_manager.reset(
            new EquationOfShearViscosityMixingRulesManager(
                "d_equation_of_shear_viscosity_mixing_rules_manager",
                d_dim,
                d_num_species,
                MIXING_CLOSURE_MODEL::ISOTHERMAL_AND_ISOBARIC,
                equation_of_shear_viscosity_mixing_rules_db,
                d_equation_of_shear_viscosity_str));
        
        d_equation_of_shear_viscosity_mixing_rules =
            d_equation_of_shear_viscosity_mixing_rules_manager->
                getEquationOfShearViscosityMixingRules();
    }
    
    /*
     * Initialize d_equation_of_bulk_viscosity_mixing_rules_manager and get the equation of
     * bulk viscosity mixing rules object.
     */
    
    if ((flow_model_db->keyExists("equation_of_bulk_viscosity")) ||
        (flow_model_db->keyExists("d_equation_of_bulk_viscosity_str")))
    {
        if (flow_model_db->keyExists("equation_of_bulk_viscosity"))
        {
            d_equation_of_bulk_viscosity_str =
                flow_model_db->getString("equation_of_bulk_viscosity");
        }
        else if (flow_model_db->keyExists("d_equation_of_bulk_viscosity_str"))
        {
            d_equation_of_bulk_viscosity_str =
                flow_model_db->getString("d_equation_of_bulk_viscosity_str");
        }
        
        boost::shared_ptr<tbox::Database> equation_of_bulk_viscosity_mixing_rules_db;
        
        if (flow_model_db->keyExists("Equation_of_bulk_viscosity_mixing_rules"))
        {
            equation_of_bulk_viscosity_mixing_rules_db =
                flow_model_db->getDatabase("Equation_of_bulk_viscosity_mixing_rules");
        }
        else if (flow_model_db->keyExists("d_equation_of_bulk_viscosity_mixing_rules_db"))
        {
            equation_of_bulk_viscosity_mixing_rules_db =
                flow_model_db->getDatabase("d_equation_of_bulk_viscosity_mixing_rules_db");
        }
        else
        {
            TBOX_ERROR(d_object_name
                << ": "
                << "No key 'Equation_of_bulk_viscosity_mixing_rules'/"
                << "'d_equation_of_bulk_viscosity_mixing_rules_db' found in data for flow model"
                << std::endl);
        }
        
        d_equation_of_bulk_viscosity_mixing_rules_manager.reset(
            new EquationOfBulkViscosityMixingRulesManager(
                "d_equation_of_bulk_viscosity_mixing_rules_manager",
                d_dim,
                d_num_species,
                MIXING_CLOSURE_MODEL::ISOTHERMAL_AND_ISOBARIC,
                equation_of_bulk_viscosity_mixing_rules_db,
                d_equation_of_bulk_viscosity_str));
        
        d_equation_of_bulk_viscosity_mixing_rules =
            d_equation_of_bulk_viscosity_mixing_rules_manager->
                getEquationOfBulkViscosityMixingRules();
    }
    
    /*
     * Initialize d_equation_of_thermal_conductivity_mixing_rules_manager and get the equation of
     * thermal conductivity mixing rules object.
     */
    
    if ((flow_model_db->keyExists("equation_of_thermal_conductivity")) ||
        (flow_model_db->keyExists("d_equation_of_thermal_conductivity_str")))
    {
        if (flow_model_db->keyExists("equation_of_thermal_conductivity"))
        {
            d_equation_of_thermal_conductivity_str =
                flow_model_db->getString("equation_of_thermal_conductivity");
        }
        else if (flow_model_db->keyExists("d_equation_of_thermal_conductivity_str"))
        {
            d_equation_of_thermal_conductivity_str =
                flow_model_db->getString("d_equation_of_thermal_conductivity_str");
        }
        
        boost::shared_ptr<tbox::Database> equation_of_thermal_conductivity_mixing_rules_db;
        
        if (flow_model_db->keyExists("Equation_of_thermal_conductivity_mixing_rules"))
        {
            equation_of_thermal_conductivity_mixing_rules_db =
                flow_model_db->getDatabase("Equation_of_thermal_conductivity_mixing_rules");
        }
        else if (flow_model_db->keyExists("d_equation_of_thermal_conductivity_mixing_rules_db"))
        {
            equation_of_thermal_conductivity_mixing_rules_db =
                flow_model_db->getDatabase("d_equation_of_thermal_conductivity_mixing_rules_db");
        }
        else
        {
            TBOX_ERROR(d_object_name
                << ": "
                << "No key 'Equation_of_thermal_conductivity_mixing_rules'/"
                << "'d_equation_of_thermal_conductivity_mixing_rules_db' found in data for flow model"
                << std::endl);
        }
        
        d_equation_of_thermal_conductivity_mixing_rules_manager.reset(
            new EquationOfThermalConductivityMixingRulesManager(
                "d_equation_of_thermal_conductivity_mixing_rules_manager",
                d_dim,
                d_num_species,
                MIXING_CLOSURE_MODEL::ISOTHERMAL_AND_ISOBARIC,
                equation_of_thermal_conductivity_mixing_rules_db,
                d_equation_of_thermal_conductivity_str));
        
        d_equation_of_thermal_conductivity_mixing_rules =
            d_equation_of_thermal_conductivity_mixing_rules_manager->
                getEquationOfThermalConductivityMixingRules();
    }
    
    /*
     * Initialize the Riemann solvers.
     */
    d_Riemann_solver_HLLC = boost::shared_ptr<RiemannSolverFourEqnConservativeHLLC> (
        new RiemannSolverFourEqnConservativeHLLC(
            d_object_name,
            d_dim,
            d_num_eqn,
            d_num_species,
            d_equation_of_state_mixing_rules));
    
    d_Riemann_solver_HLLC_HLL = boost::shared_ptr<RiemannSolverFourEqnConservativeHLLC_HLL> (
        new RiemannSolverFourEqnConservativeHLLC_HLL(
            d_object_name,
            d_dim,
            d_num_eqn,
            d_num_species,
            d_equation_of_state_mixing_rules));
    
    d_flow_model_boundary_utilities.reset(
        new FlowModelBoundaryUtilitiesFourEqnConservative(
            "d_flow_model_boundary_utilities",
            d_dim,
            d_num_species,
            d_num_eqn,
            d_equation_of_state_mixing_rules));
}


/*
 * Print all characteristics of the flow model class.
 */
void
FlowModelFourEqnConservative::printClassData(std::ostream& os) const
{
    os << "\nPrint FlowModelFourEqnConservative object..."
       << std::endl;
    
    os << std::endl;
    
    os << "FlowModelFourEqnConservative: this = "
       << (FlowModelFourEqnConservative *)this
       << std::endl;
    
    os << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
    d_equation_of_state_mixing_rules_manager->printClassData(os);
}


/*
 * Put the characteristics of the flow model class into the restart database.
 */
void
FlowModelFourEqnConservative::putToRestart(
    const boost::shared_ptr<tbox::Database>& restart_db) const
{
    /*
     * Put the properties of d_equation_of_state_mixing_rules into the restart database.
     */
    
    restart_db->putString("d_equation_of_state_str", d_equation_of_state_str);
    
    boost::shared_ptr<tbox::Database> restart_equation_of_state_mixing_rules_db =
        restart_db->putDatabase("d_equation_of_state_mixing_rules_db");
    d_equation_of_state_mixing_rules->putToRestart(restart_equation_of_state_mixing_rules_db);
    
    /*
     * Put the properties of d_equation_of_mass_diffusivity_mixing_rules into the restart database.
     */
    
    if (d_equation_of_mass_diffusivity_mixing_rules)
    {
        restart_db->putString("d_equation_of_mass_diffusivity_str", d_equation_of_mass_diffusivity_str);
        
        boost::shared_ptr<tbox::Database> restart_equation_of_mass_diffusivity_mixing_rules_db =
            restart_db->putDatabase("d_equation_of_mass_diffusivity_mixing_rules_db");
        d_equation_of_mass_diffusivity_mixing_rules->
            putToRestart(restart_equation_of_mass_diffusivity_mixing_rules_db);
    }
    
    /*
     * Put the properties of d_equation_of_shear_viscosity_mixing_rules into the restart database.
     */
    
    if (d_equation_of_shear_viscosity_mixing_rules)
    {
        restart_db->putString("d_equation_of_shear_viscosity_str", d_equation_of_shear_viscosity_str);
        
        boost::shared_ptr<tbox::Database> restart_equation_of_shear_viscosity_mixing_rules_db =
            restart_db->putDatabase("d_equation_of_shear_viscosity_mixing_rules_db");
        d_equation_of_shear_viscosity_mixing_rules->
            putToRestart(restart_equation_of_shear_viscosity_mixing_rules_db);
    }
    
    /*
     * Put the properties of d_equation_of_bulk_viscosity_mixing_rules into the restart database.
     */
    
    if (d_equation_of_bulk_viscosity_mixing_rules)
    {
        restart_db->putString("d_equation_of_bulk_viscosity_str", d_equation_of_bulk_viscosity_str);
        
        boost::shared_ptr<tbox::Database> restart_equation_of_bulk_viscosity_mixing_rules_db =
            restart_db->putDatabase("d_equation_of_bulk_viscosity_mixing_rules_db");
        d_equation_of_bulk_viscosity_mixing_rules->
            putToRestart(restart_equation_of_bulk_viscosity_mixing_rules_db);
    }
    
    /*
     * Put the properties of d_equation_of_thermal_conductivity_mixing_rules into the restart database.
     */
    
    if (d_equation_of_thermal_conductivity_mixing_rules)
    {
        restart_db->putString("d_equation_of_thermal_conductivity_str", d_equation_of_thermal_conductivity_str);
        
        boost::shared_ptr<tbox::Database> restart_equation_of_thermal_conductivity_mixing_rules_db =
            restart_db->putDatabase("d_equation_of_thermal_conductivity_mixing_rules_db");
        d_equation_of_thermal_conductivity_mixing_rules->
            putToRestart(restart_equation_of_thermal_conductivity_mixing_rules_db);
    }
}


/*
 * Register the conservative variables.
 */
void
FlowModelFourEqnConservative::registerConservativeVariables(RungeKuttaLevelIntegrator* integrator)
{
    integrator->registerVariable(
        d_variable_partial_density,
        d_num_ghosts,
        RungeKuttaLevelIntegrator::TIME_DEP,
        d_grid_geometry,
        "CONSERVATIVE_COARSEN",
        "CONSERVATIVE_LINEAR_REFINE");
    
    integrator->registerVariable(
        d_variable_momentum,
        d_num_ghosts,
        RungeKuttaLevelIntegrator::TIME_DEP,
        d_grid_geometry,
        "CONSERVATIVE_COARSEN",
        "CONSERVATIVE_LINEAR_REFINE");
    
    integrator->registerVariable(
        d_variable_total_energy,
        d_num_ghosts,
        RungeKuttaLevelIntegrator::TIME_DEP,
        d_grid_geometry,
        "CONSERVATIVE_COARSEN",
        "CONSERVATIVE_LINEAR_REFINE");
}


/*
 * Get the names of conservative variables.
 */
std::vector<std::string>
FlowModelFourEqnConservative::getNamesOfConservativeVariables(bool have_underscores)
{
    std::vector<std::string> names;
    names.reserve(3);
    
    if (have_underscores)
    {
        names.push_back("partial_density");
        names.push_back("momentum");
        names.push_back("total_energy");
    }
    else
    {
        names.push_back("partial density");
        names.push_back("momentum");
        names.push_back("total energy");
    }
    
    return names;
}


/*
 * Get the names of primitive variables.
 */
std::vector<std::string> FlowModelFourEqnConservative::getNamesOfPrimitiveVariables(bool have_underscores)
{
    std::vector<std::string> names;
    names.reserve(3);
    
    if (have_underscores)
    {
        names.push_back("partial_density");
        names.push_back("velocity");
        names.push_back("pressure");
    }
    else
    {
        names.push_back("partial density");
        names.push_back("velocity");
        names.push_back("pressure");
    }
    
    return names;
}


/*
 * Get the variable types of conservative variables.
 */
std::vector<std::string>
FlowModelFourEqnConservative::getVariableTypesOfConservativeVariables()
{
    std::vector<std::string> types;
    types.reserve(3);
    
    types.push_back("SCALAR");
    types.push_back("VECTOR");
    types.push_back("SCALAR");
    
    return types;
}


/*
 * Get the variable types of primitive variables.
 */
std::vector<std::string>
FlowModelFourEqnConservative::getVariableTypesOfPrimitiveVariables()
{
    std::vector<std::string> types;
    types.reserve(3);
    
    types.push_back("SCALAR");
    types.push_back("VECTOR");
    types.push_back("SCALAR");
    
    return types;
}


/*
 * Get the conservative variables.
 */
std::vector<boost::shared_ptr<pdat::CellVariable<double> > >
FlowModelFourEqnConservative::getConservativeVariables()
{
    std::vector<boost::shared_ptr<pdat::CellVariable<double> > > conservative_variables;
    conservative_variables.reserve(3);
    
    conservative_variables.push_back(d_variable_partial_density);
    conservative_variables.push_back(d_variable_momentum);
    conservative_variables.push_back(d_variable_total_energy);
    
    return conservative_variables;
}


/*
 * Register a patch with a data context.
 */
void
FlowModelFourEqnConservative::registerPatchWithDataContext(
    const hier::Patch& patch,
    const boost::shared_ptr<hier::VariableContext>& data_context)
{
    // Check whether the patch is already unregistered.
    if (d_patch)
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::registerPatchWithDataContext()\n"
            << "The patch is not yet unregistered."
            << std::endl);
    }
    
    d_patch = &patch;
    
    setDataContext(data_context);
    
    /*
     * Set the interior and ghost boxes with their dimensions for the conservative cell variables.
     */
    
    d_interior_box = d_patch->getBox();
    d_interior_dims = d_interior_box.numberCells();
    
    d_ghost_box = d_interior_box;
    d_ghost_box.grow(d_num_ghosts);
    d_ghostcell_dims = d_ghost_box.numberCells();
}


/*
 * Register different derived variables in the registered patch. The derived variables to be registered
 * are given as entires in a map of the variable name to the number of sub-ghost cells required.
 * If the variable to be registered is one of the conservative variable, the corresponding entry
 * in the map is ignored.
 */
void
FlowModelFourEqnConservative::registerDerivedCellVariable(
    const std::unordered_map<std::string, hier::IntVector>& num_subghosts_of_data)
{
    // Check whether a patch is already registered.
    if (!d_patch)
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::registerDerivedCellVariable()\n"
            << "No patch is registered yet."
            << std::endl);
    }
    
    for (std::unordered_map<std::string, hier::IntVector>::const_iterator it = num_subghosts_of_data.begin();
         it != num_subghosts_of_data.end();
         it++)
    {
        if ((it->second < hier::IntVector::getZero(d_dim)) ||
            (it->second > d_num_ghosts))
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::registerDerivedCellVariable()\n"
                << "The number of sub-ghost cells of variables '"
                << it->first
                << "' is not between zero and d_num_ghosts."
                << std::endl);
        }
    }
    
    if (num_subghosts_of_data.find("DENSITY") != num_subghosts_of_data.end())
    {
        setNumberOfSubGhosts(
            num_subghosts_of_data.find("DENSITY")->second,
            "DENSITY",
            "DENSITY");
    }
    
    if (num_subghosts_of_data.find("MASS_FRACTION") != num_subghosts_of_data.end())
    {
        setNumberOfSubGhosts(
            num_subghosts_of_data.find("MASS_FRACTION")->second,
            "MASS_FRACTION",
            "MASS_FRACTION");
    }
    
    if (num_subghosts_of_data.find("VELOCITY") != num_subghosts_of_data.end())
    {
        setNumberOfSubGhosts(
            num_subghosts_of_data.find("VELOCITY")->second,
            "VELOCITY",
            "VELOCITY");
    }
    
    if (num_subghosts_of_data.find("INTERNAL_ENERGY") != num_subghosts_of_data.end())
    {
        setNumberOfSubGhosts(
            num_subghosts_of_data.find("INTERNAL_ENERGY")->second,
            "INTERNAL_ENERGY",
            "INTERNAL_ENERGY");
    }
    
    if (num_subghosts_of_data.find("PRESSURE") != num_subghosts_of_data.end())
    {
        setNumberOfSubGhosts(
            num_subghosts_of_data.find("PRESSURE")->second,
            "PRESSURE",
            "PRESSURE");
    }
    
    if (num_subghosts_of_data.find("SOUND_SPEED") != num_subghosts_of_data.end())
    {
        setNumberOfSubGhosts(
            num_subghosts_of_data.find("SOUND_SPEED")->second,
            "SOUND_SPEED",
            "SOUND_SPEED");
    }
    
    if (num_subghosts_of_data.find("TEMPERATURE") != num_subghosts_of_data.end())
    {
        setNumberOfSubGhosts(
            num_subghosts_of_data.find("TEMPERATURE")->second,
            "TEMPERATURE",
            "TEMPERATURE");
    }
    
    if (num_subghosts_of_data.find("DILATATION") != num_subghosts_of_data.end())
    {
        setNumberOfSubGhosts(
            num_subghosts_of_data.find("DILATATION")->second,
            "DILATATION",
            "DILATATION");
    }
    
    if (num_subghosts_of_data.find("VORTICITY") != num_subghosts_of_data.end())
    {
        setNumberOfSubGhosts(
            num_subghosts_of_data.find("VORTICITY")->second,
            "VORTICITY",
            "VORTICITY");
    }
    
    if (num_subghosts_of_data.find("ENSTROPHY") != num_subghosts_of_data.end())
    {
        setNumberOfSubGhosts(
            num_subghosts_of_data.find("ENSTROPHY")->second,
            "ENSTROPHY",
            "ENSTROPHY");
    }
    
    if (num_subghosts_of_data.find("CONVECTIVE_FLUX_X") != num_subghosts_of_data.end())
    {
        setNumberOfSubGhosts(
            num_subghosts_of_data.find("CONVECTIVE_FLUX_X")->second,
            "CONVECTIVE_FLUX_X",
            "CONVECTIVE_FLUX_X");
    }
    
    if (num_subghosts_of_data.find("CONVECTIVE_FLUX_Y") != num_subghosts_of_data.end())
    {
        setNumberOfSubGhosts(
            num_subghosts_of_data.find("CONVECTIVE_FLUX_Y")->second,
            "CONVECTIVE_FLUX_Y",
            "CONVECTIVE_FLUX_Y");
    }
    
    if (num_subghosts_of_data.find("CONVECTIVE_FLUX_Z") != num_subghosts_of_data.end())
    {
        setNumberOfSubGhosts(
            num_subghosts_of_data.find("CONVECTIVE_FLUX_Z")->second,
            "CONVECTIVE_FLUX_Z",
            "CONVECTIVE_FLUX_Z");
    }
    
    if (num_subghosts_of_data.find("PRIMITIVE_VARIABLES") != num_subghosts_of_data.end())
    {
        setNumberOfSubGhosts(
            num_subghosts_of_data.find("PRIMITIVE_VARIABLES")->second,
            "PRIMITIVE_VARIABLES",
            "PRIMITIVE_VARIABLES");
    }
    
    if (num_subghosts_of_data.find("MAX_WAVE_SPEED_X") != num_subghosts_of_data.end())
    {
        setNumberOfSubGhosts(
            num_subghosts_of_data.find("MAX_WAVE_SPEED_X")->second,
            "MAX_WAVE_SPEED_X",
            "MAX_WAVE_SPEED_X");
    }
    
    if (num_subghosts_of_data.find("MAX_WAVE_SPEED_Y") != num_subghosts_of_data.end())
    {
        setNumberOfSubGhosts(
            num_subghosts_of_data.find("MAX_WAVE_SPEED_Y")->second,
            "MAX_WAVE_SPEED_Y",
            "MAX_WAVE_SPEED_Y");
    }
    
    if (num_subghosts_of_data.find("MAX_WAVE_SPEED_Z") != num_subghosts_of_data.end())
    {
        setNumberOfSubGhosts(
            num_subghosts_of_data.find("MAX_WAVE_SPEED_Z")->second,
            "MAX_WAVE_SPEED_Z",
            "MAX_WAVE_SPEED_Z");
    }
}


/*
 * Register the required variables for the computation of projection matrix
 * of conservative variables and its inverse at faces in the registered patch.
 */
void
FlowModelFourEqnConservative::registerFaceProjectionMatricesOfConservativeVariables(
    const hier::IntVector& num_subghosts,
    const AVERAGING::TYPE& averaging)
{
    // Check whether a patch is already registered.
    if (!d_patch)
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::registerFaceProjectionMatricesOfConservativeVariables()\n"
            << "No patch is registered yet."
            << std::endl);
    }
    
    NULL_USE(num_subghosts);
    
    d_proj_mat_conservative_var_averaging = averaging;
    
    d_proj_mat_conservative_var_registered = true;
}


/*
 * Register the required variables for the computation of projection matrix
 * of primitive variables and its inverse at faces in the registered patch.
 */
void
FlowModelFourEqnConservative::registerFaceProjectionMatricesOfPrimitiveVariables(
    const hier::IntVector& num_subghosts,
    const AVERAGING::TYPE& averaging)
{
    // Check whether a patch is already registered.
    if (!d_patch)
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::registerFaceProjectionMatricesOfPrimitiveVariables()\n"
            << "No patch is registered yet."
            << std::endl);
    }
    
    d_proj_mat_primitive_var_averaging = averaging;
    
    setNumberOfSubGhosts(
        num_subghosts,
        "SOUND_SPEED",
        "PROJECTION_MATRICES");
    
    d_proj_mat_primitive_var_registered = true;
}


/*
 * Register the required variables for the computation of diffusive flux in the
 * registered patch.
 */
void
FlowModelFourEqnConservative::registerDiffusiveFlux(const hier::IntVector& num_subghosts)
{
    setNumberOfSubGhosts(
        num_subghosts,
        "DENSITY",
        "DIFFUSIVE_FLUX");
    
    setNumberOfSubGhosts(
        num_subghosts,
        "MASS_FRACTION",
        "DIFFUSIVE_FLUX");
    
    setNumberOfSubGhosts(
        num_subghosts,
        "VELOCITY",
        "DIFFUSIVE_FLUX");
    
    setNumberOfSubGhosts(
        num_subghosts,
        "PRESSURE",
        "DIFFUSIVE_FLUX");
    
    setNumberOfSubGhosts(
        num_subghosts,
        "TEMPERATURE",
        "DIFFUSIVE_FLUX");
    
    d_num_subghosts_diffusivities = 
        hier::IntVector::min(d_num_subghosts_density, d_num_subghosts_mass_fraction);
    
    d_num_subghosts_diffusivities =
        hier::IntVector::min(d_num_subghosts_diffusivities, d_num_subghosts_velocity);
    
    d_num_subghosts_diffusivities =
        hier::IntVector::min(d_num_subghosts_diffusivities, d_num_subghosts_pressure);
    
    d_num_subghosts_diffusivities =
        hier::IntVector::min(d_num_subghosts_diffusivities, d_num_subghosts_temperature);
    
    d_diffusive_flux_var_registered = true;
}


/*
 * Unregister the registered patch. The registered data context and all global derived
 * cell data in the patch are dumped.
 */
void
FlowModelFourEqnConservative::unregisterPatch()
{
    // Check whether a patch is already registered.
    if (!d_patch)
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::unregisterPatch()\n"
            << "No patch is registered yet."
            << std::endl);
    }
    
    d_patch = nullptr;
    
    d_num_subghosts_density           = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_mass_fraction     = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_velocity          = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_internal_energy   = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_pressure          = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_sound_speed       = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_temperature       = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_dilatation        = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_vorticity         = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_enstrophy         = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_convective_flux_x = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_convective_flux_y = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_convective_flux_z = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_max_wave_speed_x  = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_max_wave_speed_y  = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_max_wave_speed_z  = -hier::IntVector::getOne(d_dim);
    d_num_subghosts_diffusivities     = -hier::IntVector::getOne(d_dim);
    
    d_interior_box                   = hier::Box::getEmptyBox(d_dim);
    d_ghost_box                      = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_density           = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_mass_fraction     = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_velocity          = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_internal_energy   = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_pressure          = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_sound_speed       = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_temperature       = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_dilatation        = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_vorticity         = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_enstrophy         = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_convective_flux_x = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_convective_flux_y = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_convective_flux_z = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_max_wave_speed_x  = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_max_wave_speed_y  = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_max_wave_speed_z  = hier::Box::getEmptyBox(d_dim);
    d_subghost_box_diffusivities     = hier::Box::getEmptyBox(d_dim);
    
    d_interior_dims                       = hier::IntVector::getZero(d_dim);
    d_ghostcell_dims                      = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_density           = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_mass_fraction     = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_velocity          = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_internal_energy   = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_pressure          = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_sound_speed       = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_temperature       = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_dilatation        = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_vorticity         = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_enstrophy         = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_convective_flux_x = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_convective_flux_y = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_convective_flux_z = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_max_wave_speed_x  = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_max_wave_speed_y  = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_max_wave_speed_z  = hier::IntVector::getZero(d_dim);
    d_subghostcell_dims_diffusivities     = hier::IntVector::getZero(d_dim);
    
    d_data_density.reset();
    d_data_mass_fraction.reset();
    d_data_velocity.reset();
    d_data_internal_energy.reset();
    d_data_pressure.reset();
    d_data_sound_speed.reset();
    d_data_temperature.reset();
    d_data_dilatation.reset();
    d_data_vorticity.reset();
    d_data_enstrophy.reset();
    d_data_convective_flux_x.reset();
    d_data_convective_flux_y.reset();
    d_data_convective_flux_z.reset();
    d_data_max_wave_speed_x.reset();
    d_data_max_wave_speed_y.reset();
    d_data_max_wave_speed_z.reset();
    d_data_diffusivities.reset();
    
    d_proj_mat_conservative_var_registered = false;
    d_proj_mat_primitive_var_registered    = false;
    
    clearDataContext();
}


/*
 * Compute global cell data of different registered derived variables with the registered data context.
 */
void
FlowModelFourEqnConservative::computeGlobalDerivedCellData()
{
    // Check whether a patch is already registered.
    if (!d_patch)
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::computeGlobalDerivedCellData()\n"
            << "No patch is registered yet."
            << std::endl);
    }
    
    /*
     * Set the boxes and their dimensions for the derived cell variables.
     */
    setGhostBoxesAndDimensionsDerivedCellVariables();
    
    // Compute the total density cell data.
    if (d_num_subghosts_density > -hier::IntVector::getOne(d_dim))
    {
        if (!d_data_density)
        {
            computeGlobalCellDataDensity();
        }
    }
    
    // Compute the mass fraction cell data.
    if (d_num_subghosts_mass_fraction > -hier::IntVector::getOne(d_dim))
    {
        if (!d_data_mass_fraction)
        {
            computeGlobalCellDataMassFractionWithDensity();
        }
    }
    
    // Compute the velocity cell data.
    if (d_num_subghosts_velocity > -hier::IntVector::getOne(d_dim))
    {
        if (!d_data_velocity)
        {
            computeGlobalCellDataVelocityWithDensity();
        }
    }
    
    // Compute the internal energy cell data.
    if (d_num_subghosts_internal_energy > -hier::IntVector::getOne(d_dim))
    {
        if (!d_data_internal_energy)
        {
            computeGlobalCellDataInternalEnergyWithDensityAndVelocity();
        }
    }
    
    // Compute the pressure cell data.
    if (d_num_subghosts_pressure > -hier::IntVector::getOne(d_dim))
    {
        if (!d_data_pressure)
        {
            computeGlobalCellDataPressureWithDensityMassFractionAndInternalEnergy();
        }
    }
    
    // Compute the sound speed cell data.
    if (d_num_subghosts_sound_speed > -hier::IntVector::getOne(d_dim))
    {
        if (!d_data_sound_speed)
        {
            computeGlobalCellDataSoundSpeedWithDensityMassFractionAndPressure();
        }
    }
    
    // Compute the temperature cell data.
    if (d_num_subghosts_temperature > -hier::IntVector::getOne(d_dim))
    {
        if (!d_data_temperature)
        {
            computeGlobalCellDataTemperatureWithDensityMassFractionAndPressure();
        }
    }
    
    // Compute the dilatation cell data.
    if (d_num_subghosts_dilatation > -hier::IntVector::getOne(d_dim))
    {
        if (!d_data_dilatation)
        {
            computeGlobalCellDataDilatationWithDensityAndVelocity();
        }
    }
    
    // Compute the vorticity cell data.
    if (d_num_subghosts_vorticity > -hier::IntVector::getOne(d_dim))
    {
        if (!d_data_vorticity)
        {
            computeGlobalCellDataVorticityWithDensityAndVelocity();
        }
    }
    
    // Compute the enstrophy cell data.
    if (d_num_subghosts_enstrophy > -hier::IntVector::getOne(d_dim))
    {
        if (!d_data_enstrophy)
        {
            computeGlobalCellDataEnstrophyWithVorticity();
        }
    }
    
    // Compute the x-direction convective flux cell data.
    if (d_num_subghosts_convective_flux_x > -hier::IntVector::getOne(d_dim))
    {
        if (!d_data_convective_flux_x)
        {
            computeGlobalCellDataConvectiveFluxWithVelocityAndPressure(DIRECTION::X_DIRECTION);
        }
    }
    
    // Compute the y-direction convective flux cell data.
    if (d_num_subghosts_convective_flux_y > -hier::IntVector::getOne(d_dim))
    {
        if (!d_data_convective_flux_y)
        {
            computeGlobalCellDataConvectiveFluxWithVelocityAndPressure(DIRECTION::Y_DIRECTION);
        }
    }
    
    // Compute the z-direction convective flux cell data.
    if (d_num_subghosts_convective_flux_z > -hier::IntVector::getOne(d_dim))
    {
        if (!d_data_convective_flux_z)
        {
            computeGlobalCellDataConvectiveFluxWithVelocityAndPressure(DIRECTION::Z_DIRECTION);
        }
    }
    
    // Compute the x-direction maximum wave speed cell data.
    if (d_num_subghosts_max_wave_speed_x > -hier::IntVector::getOne(d_dim))
    {
        if (!d_data_max_wave_speed_x)
        {
            computeGlobalCellDataMaxWaveSpeedWithVelocityAndSoundSpeed(DIRECTION::X_DIRECTION);
        }
    }
    
    // Compute the y-direction maximum wave speed cell data.
    if (d_num_subghosts_max_wave_speed_y > -hier::IntVector::getOne(d_dim))
    {
        if (!d_data_max_wave_speed_y)
        {
            computeGlobalCellDataMaxWaveSpeedWithVelocityAndSoundSpeed(DIRECTION::Y_DIRECTION);
        }
    }
    
    // Compute the z-direction maximum wave speed cell data.
    if (d_num_subghosts_max_wave_speed_z > -hier::IntVector::getOne(d_dim))
    {
        if (!d_data_max_wave_speed_z)
        {
            computeGlobalCellDataMaxWaveSpeedWithVelocityAndSoundSpeed(DIRECTION::Z_DIRECTION);
        }
    }
}


/*
 * Get the global cell data of one cell variable in the registered patch.
 */
boost::shared_ptr<pdat::CellData<double> >
FlowModelFourEqnConservative::getGlobalCellData(const std::string& variable_key)
{
    // Check whether a patch is already registered.
    if (!d_patch)
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
            << "No patch is registered yet."
            << std::endl);
    }
    
    boost::shared_ptr<pdat::CellData<double> > cell_data;
    
    if (variable_key == "PARTIAL_DENSITY")
    {
        cell_data = getGlobalCellDataPartialDensity();
    }
    else if (variable_key == "MOMENTUM")
    {
        cell_data = getGlobalCellDataMomentum();
    }
    else if (variable_key == "TOTAL_ENERGY")
    {
        cell_data = getGlobalCellDataTotalEnergy();
    }
    else if (variable_key == "DENSITY")
    {
        if (!d_data_density)
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
                << "Cell data of 'DENSITY' is not registered/computed yet."
                << std::endl);
        }
        cell_data = d_data_density;
    }
    else if (variable_key == "MASS_FRACTION")
    {
        if (!d_data_mass_fraction)
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
                << "Cell data of 'MASS_FRACTION' is not registered/computed yet."
                << std::endl);
        }
        cell_data = d_data_mass_fraction;
    }
    else if (variable_key == "VELOCITY")
    {
        if (!d_data_velocity)
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
                << "Cell data of 'VELOCITY' is not registered/computed yet."
                << std::endl);
        }
        cell_data = d_data_velocity;
    }
    else if (variable_key == "INTERNAL_ENERGY")
    {
        if (!d_data_internal_energy)
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
                << "Cell data of 'INTERNAL_ENERGY' is not registered/computed yet."
                << std::endl);
        }
        cell_data = d_data_internal_energy;
    }
    else if (variable_key == "PRESSURE")
    {
        if (!d_data_pressure)
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
                << "Cell data of 'PRESSURE' is not registered/computed yet."
                << std::endl);
        }
        cell_data = d_data_pressure;
    }
    else if (variable_key == "SOUND_SPEED")
    {
        if (!d_data_sound_speed)
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
                << "Cell data of 'SOUND_SPEED' is not registered/computed yet."
                << std::endl);
        }
        cell_data = d_data_sound_speed;
    }
    else if (variable_key == "TEMPERATURE")
    {
        if (!d_data_temperature)
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
                << "Cell data of 'TEMPERATURE' is not registered/computed yet."
                << std::endl);
        }
        cell_data = d_data_temperature;
    }
    else if (variable_key == "DILATATION")
    {
        if (!d_data_dilatation)
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
                << "Cell data of 'DILATATION' is not registered/computed yet."
                << std::endl);
        }
        cell_data = d_data_dilatation;
    }
    else if (variable_key == "VORTICITY")
    {
        if (!d_data_vorticity)
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
                << "Cell data of 'VORTICITY' is not registered/computed yet."
                << std::endl);
        }
        cell_data = d_data_vorticity;
    }
    else if (variable_key == "ENSTROPHY")
    {
        if (!d_data_enstrophy)
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
                << "Cell data of 'ENSTROPHY' is not registered/computed yet."
                << std::endl);
        }
        cell_data = d_data_enstrophy;
    }
    else if (variable_key == "CONVECTIVE_FLUX_X")
    {
        if (!d_data_convective_flux_x)
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
                << "Cell data of 'CONVECTIVE_FLUX_X' is not registered/computed yet."
                << std::endl);
        }
        cell_data = d_data_convective_flux_x;
    }
    else if (variable_key == "CONVECTIVE_FLUX_Y")
    {
        if (!d_data_convective_flux_y)
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
                << "Cell data of 'CONVECTIVE_FLUX_Y' is not registered/computed yet."
                << std::endl);
        }
        cell_data = d_data_convective_flux_y;
    }
    else if (variable_key == "CONVECTIVE_FLUX_Z")
    {
        if (!d_data_convective_flux_z)
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
                << "Cell data of 'CONVECTIVE_FLUX_Z' is not registered/computed yet."
                << std::endl);
        }
        cell_data = d_data_convective_flux_z;
    }
    else if (variable_key == "MAX_WAVE_SPEED_X")
    {
        if (!d_data_max_wave_speed_x)
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
                << "Cell data of 'MAX_WAVE_SPEED_X' is not registered/computed yet."
                << std::endl);
        }
        cell_data = d_data_max_wave_speed_x;
    }
    else if (variable_key == "MAX_WAVE_SPEED_Y")
    {
        if (!d_data_max_wave_speed_y)
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
                << "Cell data of 'MAX_WAVE_SPEED_Y' is not registered/computed yet."
                << std::endl);
        }
        cell_data = d_data_max_wave_speed_y;
    }
    else if (variable_key == "MAX_WAVE_SPEED_Z")
    {
        if (!d_data_max_wave_speed_z)
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
                << "Cell data of 'MAX_WAVE_SPEED_Z' is not registered/computed yet."
                << std::endl);
        }
        cell_data = d_data_max_wave_speed_z;
    }
    else
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::getGlobalCellData()\n"
            << "Unknown cell data with variable_key = '" << variable_key
            << "' requested."
            << std::endl);
    }
    
    return cell_data;
}


/*
 * Get the global cell data of different cell variables in the registered patch.
 */
std::vector<boost::shared_ptr<pdat::CellData<double> > >
FlowModelFourEqnConservative::getGlobalCellData(
    const std::vector<std::string>& variable_keys)
{
    std::vector<boost::shared_ptr<pdat::CellData<double> > > cell_data(
        static_cast<int>(variable_keys.size()));
    
    for (int vi = 0; static_cast<int>(variable_keys.size()); vi++)
    {
        cell_data[vi] = getGlobalCellData(variable_keys[vi]);
    }
    
    return cell_data;
}


/*
 * Fill the interior global cell data of conservative variables with zeros.
 */
void
FlowModelFourEqnConservative::fillZeroGlobalCellDataConservativeVariables()
{
    // Check whether a patch is already registered.
    if (!d_patch)
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::fillZeroGlobalCellDataConservativeVariables()\n"
            << "No patch is registered yet."
            << std::endl);
    }
    
    boost::shared_ptr<pdat::CellData<double> > data_partial_density = getGlobalCellDataPartialDensity();
    boost::shared_ptr<pdat::CellData<double> > data_momentum = getGlobalCellDataMomentum();
    boost::shared_ptr<pdat::CellData<double> > data_total_energy = getGlobalCellDataTotalEnergy();
    
    data_partial_density->fillAll(0.0, d_interior_box);
    data_momentum->fillAll(0.0, d_interior_box);
    data_total_energy->fillAll(0.0, d_interior_box);
}


/*
 * Update the interior global cell data of conservative variables.
 */
void
FlowModelFourEqnConservative::updateGlobalCellDataConservativeVariables()
{
    // Check whether a patch is already registered.
    if (!d_patch)
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::updateGlobalCellDataConservativeVariables()\n"
            << "No patch is registered yet."
            << std::endl);
    }
}


/*
 * Get the global cell data of the conservative variables in the registered patch.
 */
std::vector<boost::shared_ptr<pdat::CellData<double> > >
FlowModelFourEqnConservative::getGlobalCellDataConservativeVariables()
{
    // Check whether a patch is already registered.
    if (!d_patch)
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::getGlobalCellDataConservativeVariables()\n"
            << "No patch is registered yet."
            << std::endl);
    }
    
    std::vector<boost::shared_ptr<pdat::CellData<double> > > global_cell_data;
    global_cell_data.reserve(3);
    
    global_cell_data.push_back(getGlobalCellDataPartialDensity());
    global_cell_data.push_back(getGlobalCellDataMomentum());
    global_cell_data.push_back(getGlobalCellDataTotalEnergy());
    
    return global_cell_data;
}


/*
 * Get the global cell data of the primitive variables in the registered patch.
 */
std::vector<boost::shared_ptr<pdat::CellData<double> > >
FlowModelFourEqnConservative::getGlobalCellDataPrimitiveVariables()
{
    // Check whether a patch is already registered.
    if (!d_patch)
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::getGlobalCellDataPrimitiveVariables()\n"
            << "No patch is registered yet."
            << std::endl);
    }
    
    std::vector<boost::shared_ptr<pdat::CellData<double> > > global_cell_data;
    global_cell_data.reserve(3);
    
    global_cell_data.push_back(getGlobalCellDataPartialDensity());
    if (!d_data_velocity)
    {
        computeGlobalCellDataVelocityWithDensity();
    }
    global_cell_data.push_back(d_data_velocity);
    if (!d_data_pressure)
    {
        computeGlobalCellDataPressureWithDensityMassFractionAndInternalEnergy();
    }
    global_cell_data.push_back(d_data_pressure);
    
    return global_cell_data;
}


/*
 * Get the number of projection variables for transformation between conservative
 * variables and characteristic variables.
 */
int
FlowModelFourEqnConservative::getNumberOfProjectionVariablesForConservativeVariables() const
{
    return 0;
}

/*
 * Get the number of projection variables for transformation between primitive variables
 * and characteristic variables.
 */
int
FlowModelFourEqnConservative::getNumberOfProjectionVariablesForPrimitiveVariables() const
{
    return 0;
}


/*
 * Compute global side data of the projection variables for transformation between
 * conservative variables and characteristic variables.
 */
void
FlowModelFourEqnConservative::computeGlobalSideDataProjectionVariablesForConservativeVariables(
    std::vector<boost::shared_ptr<pdat::SideData<double> > >& projection_variables)
{
    
}


/*
 * Compute global side data of the projection variables for transformation between
 * primitive variables and characteristic variables.
 */
void
FlowModelFourEqnConservative::computeGlobalSideDataProjectionVariablesForPrimitiveVariables(
    std::vector<boost::shared_ptr<pdat::SideData<double> > >& projection_variables)
{
    
}


/*
 * Compute global side data of characteristic variables from conservative variables.
 */
void
FlowModelFourEqnConservative::computeGlobalSideDataCharacteristicVariablesFromConservativeVariables(
    std::vector<boost::shared_ptr<pdat::SideData<double> > >& characteristic_variables,
    const std::vector<boost::shared_ptr<pdat::CellData<double> > >& conservative_variables,
    const std::vector<boost::shared_ptr<pdat::SideData<double> > >& projection_variables,
    const int& idx_offset)
{
    
}


/*
 * Compute global side data of characteristic variables from primitive variables.
 */
void
FlowModelFourEqnConservative::computeGlobalSideDataCharacteristicVariablesFromPrimitiveVariables(
    std::vector<boost::shared_ptr<pdat::SideData<double> > >& characteristic_variables,
    const std::vector<boost::shared_ptr<pdat::CellData<double> > >& primitive_variables,
    const std::vector<boost::shared_ptr<pdat::SideData<double> > >& projection_variables,
    const int& idx_offset)
{
    
}


/*
 * Compute global side data of conservative variables from characteristic variables.
 */
void
FlowModelFourEqnConservative::computeGlobalSideDataConservativeVariablesFromCharacteristicVariables(
    std::vector<boost::shared_ptr<pdat::SideData<double> > >& conservative_variables,
    const std::vector<boost::shared_ptr<pdat::SideData<double> > >& characteristic_variables,
    const std::vector<boost::shared_ptr<pdat::SideData<double> > >& projection_variables)
{
    
}


/*
 * Compute global side data of primitive variables from characteristic variables.
 */
void
FlowModelFourEqnConservative::computeGlobalSideDataPrimitiveVariablesFromCharacteristicVariables(
    std::vector<boost::shared_ptr<pdat::SideData<double> > >& primitive_variables,
    const std::vector<boost::shared_ptr<pdat::SideData<double> > >& characteristic_variables,
    const std::vector<boost::shared_ptr<pdat::SideData<double> > >& projection_variables)
{
    
}


/*
 * Compute the local face data of projection matrix of conservative variables in the
 * registered patch.
 */
void
FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixOfConservativeVariables(
    boost::multi_array<double, 2>& projection_matrix,
    const hier::Index& cell_index_minus,
    const hier::Index& cell_index_plus,
    const DIRECTION::TYPE& direction)
{
    NULL_USE(projection_matrix);
    NULL_USE(cell_index_minus);
    NULL_USE(cell_index_plus);
    NULL_USE(direction);
    
    TBOX_ERROR(d_object_name
        << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixOfConservativeVariables()\n"
        << "Method computeLocalFaceProjectionMatrixOfConservativeVariables() is not yet implemented."
        << std::endl);
}


/*
 * Compute the local face data of inverse of projection matrix of conservative variables
 * in the registered patch.
 */
void
FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixInverseOfConservativeVariables(
    boost::multi_array<double, 2>& projection_matrix_inv,
    const hier::Index& cell_index_minus,
    const hier::Index& cell_index_plus,
    const DIRECTION::TYPE& direction)
{
    NULL_USE(projection_matrix_inv);
    NULL_USE(cell_index_minus);
    NULL_USE(cell_index_plus);
    NULL_USE(direction);
    
    TBOX_ERROR(d_object_name
        << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixInverseOfConservativeVariables()\n"
        << "Method computeLocalFaceProjectionMatrixInverseOfConservativeVariables() is not yet implemented."
        << std::endl);
}


/*
 * Compute the local face data of projection matrix of primitive variables in the
 * registered patch.
 */
void
FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixOfPrimitiveVariables(
    boost::multi_array<double, 2>& projection_matrix,
    const hier::Index& cell_index_minus,
    const hier::Index& cell_index_plus,
    const DIRECTION::TYPE& direction)
{
    if (!d_proj_mat_primitive_var_registered)
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixOfPrimitiveVariables()\n"
            << "Projection matrices is not yet registered."
            << std::endl);
    }
    
    projection_matrix.resize(boost::extents[d_num_eqn][d_num_eqn]);
    
    // Get the cell data of the variable partial density.
    boost::shared_ptr<pdat::CellData<double> > data_partial_density =
        getGlobalCellDataPartialDensity();
    
    // Get the pointers to the cell data of partial density, total density and sound speed.
    std::vector<double*> rho_Y;
    rho_Y.reserve(d_num_species);
    for (int si = 0; si < d_num_species; si++)
    {
        rho_Y.push_back(data_partial_density->getPointer(si));
    }
    if (!d_data_sound_speed)
    {
        computeGlobalCellDataSoundSpeedWithDensityMassFractionAndPressure();
    }
    double* rho = d_data_density->getPointer(0);
    double* c = d_data_sound_speed->getPointer(0);
    
    /*
     * Fill the projection matrix with zeros.
     */
    for (int ei = 0; ei < d_num_eqn; ei++)
    {
        for (int ej = 0; ej < d_num_eqn; ej++)
        {
            projection_matrix[ei][ej] = 0.0;
        }
    }
    
    // Compute the projection matrix.
    if (d_dim == tbox::Dimension(1))
    {
        // Compute the linear indices.
        const int idx_minus = cell_index_minus[0] + d_num_ghosts[0];
        const int idx_plus = cell_index_plus[0] + d_num_ghosts[0];
        const int idx_density_minus = cell_index_minus[0] + d_num_subghosts_density[0];
        const int idx_density_plus = cell_index_plus[0] + d_num_subghosts_density[0];
        const int idx_sound_speed_minus = cell_index_minus[0] + d_num_subghosts_sound_speed[0];
        const int idx_sound_speed_plus = cell_index_plus[0] + d_num_subghosts_sound_speed[0];
        
        // Compute the average values.
        double rho_average, c_average;
        std::vector<double> rho_Y_average;
        rho_Y_average.reserve(d_num_species);
        switch (d_proj_mat_primitive_var_averaging)
        {
            case AVERAGING::SIMPLE:
            {
                rho_average = 0.5*(rho[idx_density_minus] + rho[idx_density_plus]);
                c_average = 0.5*(c[idx_sound_speed_minus] + c[idx_sound_speed_plus]);
                
                for (int si = 0; si < d_num_species; si++)
                {
                    rho_Y_average.push_back(0.5*(rho_Y[si][idx_minus] +
                        rho_Y[si][idx_plus]));
                }
                
                break;
            }
            case AVERAGING::ROE:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixOfPrimitiveVariables()\n"
                    << "Roe averaging is not yet implemented."
                    << std::endl);
                
                rho_average = 0.0;
                c_average   = 0.0;
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixOfPrimitiveVariables()\n"
                    << "Unknown d_proj_mat_primitive_var_averaging given."
                    << std::endl);
                
                rho_average = 0.0;
                c_average   = 0.0;
            }
        }
        
        switch (direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                projection_matrix[0][d_num_species] = 1.0;
                projection_matrix[0][d_num_species + 1] = -1.0/(rho_average*c_average);
                
                for (int si = 0; si < d_num_species; si++)
                {
                    projection_matrix[1 + si][si] = 1.0;
                    projection_matrix[1 + si][d_num_species + 1] = -rho_Y_average[si]/
                        (rho_average*c_average*c_average);
                }
                
                projection_matrix[d_num_species + 1][d_num_species] = 1.0;
                projection_matrix[d_num_species + 1][d_num_species + 1] = 1.0/(rho_average*c_average);
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixOfPrimitiveVariables()\n"
                << "There is only x-direction for one-dimensional problem."
                << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(2))
    {
        // Compute the linear indices.
        const int idx_minus = (cell_index_minus[0] + d_num_ghosts[0]) +
            (cell_index_minus[1] + d_num_ghosts[1])*d_ghostcell_dims[0];
        
        const int idx_plus = (cell_index_plus[0] + d_num_ghosts[0]) +
            (cell_index_plus[1] + d_num_ghosts[1])*d_ghostcell_dims[0];
        
        const int idx_density_minus = (cell_index_minus[0] + d_num_subghosts_density[0]) +
            (cell_index_minus[1] + d_num_subghosts_density[1])*d_subghostcell_dims_density[0];
        
        const int idx_density_plus = (cell_index_plus[0] + d_num_subghosts_density[0]) +
            (cell_index_plus[1] + d_num_subghosts_density[1])*d_subghostcell_dims_density[0];
        
        const int idx_sound_speed_minus = (cell_index_minus[0] + d_num_subghosts_sound_speed[0]) +
            (cell_index_minus[1] + d_num_subghosts_sound_speed[1])*d_subghostcell_dims_sound_speed[0];
        
        const int idx_sound_speed_plus = (cell_index_plus[0] + d_num_subghosts_sound_speed[0]) +
            (cell_index_plus[1] + d_num_subghosts_sound_speed[1])*d_subghostcell_dims_sound_speed[0];
        
        // Compute the average values.
        double rho_average, c_average;
        std::vector<double> rho_Y_average;
        rho_Y_average.reserve(d_num_species);
        switch (d_proj_mat_primitive_var_averaging)
        {
            case AVERAGING::SIMPLE:
            {
                rho_average = 0.5*(rho[idx_density_minus] + rho[idx_density_plus]);
                c_average = 0.5*(c[idx_sound_speed_minus] + c[idx_sound_speed_plus]);
                
                for (int si = 0; si < d_num_species; si++)
                {
                    rho_Y_average.push_back(0.5*(rho_Y[si][idx_minus] +
                        rho_Y[si][idx_plus]));
                }
                
                break;
            }
            case AVERAGING::ROE:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixOfPrimitiveVariables()\n"
                    << "Roe averaging is not yet implemented."
                    << std::endl);
                
                rho_average = 0.0;
                c_average   = 0.0;
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixOfPrimitiveVariables()\n"
                    << "Unknown d_proj_mat_primitive_var_averaging given."
                    << std::endl);
                
                rho_average = 0.0;
                c_average   = 0.0;
            }
        }
        
        switch (direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                projection_matrix[0][d_num_species] = 1.0;
                projection_matrix[0][d_num_species + 2] = -1.0/(rho_average*c_average);
                
                for (int si = 0; si < d_num_species; si++)
                {
                    projection_matrix[1 + si][si] = 1.0;
                    projection_matrix[1 + si][d_num_species + 2] = -rho_Y_average[si]/
                        (rho_average*c_average*c_average);
                }
                
                projection_matrix[d_num_species + 1][d_num_species + 1] = 1.0;
                
                projection_matrix[d_num_species + 2][d_num_species] = 1.0;
                projection_matrix[d_num_species + 2][d_num_species + 2] = 1.0/(rho_average*c_average);
                
                break;
            }
            case DIRECTION::Y_DIRECTION:
            {
                projection_matrix[0][d_num_species + 1] = 1.0;
                projection_matrix[0][d_num_species + 2] = -1.0/(rho_average*c_average);
                
                for (int si = 0; si < d_num_species; si++)
                {
                    projection_matrix[1 + si][si] = 1.0;
                    projection_matrix[1 + si][d_num_species + 2] = -rho_Y_average[si]/
                        (rho_average*c_average*c_average);
                }
                
                projection_matrix[d_num_species + 1][d_num_species] = 1.0;
                
                projection_matrix[d_num_species + 2][d_num_species + 1] = 1.0;
                projection_matrix[d_num_species + 2][d_num_species + 2] = 1.0/(rho_average*c_average);
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixOfPrimitiveVariables()\n"
                << "There are only x-direction and y-direction for two-dimensional problem."
                << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(3))
    {
        const int idx_minus = (cell_index_minus[0] + d_num_ghosts[0]) +
            (cell_index_minus[1] + d_num_ghosts[1])*d_ghostcell_dims[0] +
            (cell_index_minus[2] + d_num_ghosts[2])*d_ghostcell_dims[0]*
                d_ghostcell_dims[1];
        
        const int idx_plus = (cell_index_plus[0] + d_num_ghosts[0]) +
            (cell_index_plus[1] + d_num_ghosts[1])*d_ghostcell_dims[0] +
            (cell_index_plus[2] + d_num_ghosts[2])*d_ghostcell_dims[0]*
                d_ghostcell_dims[1];
        
        const int idx_density_minus = (cell_index_minus[0] + d_num_subghosts_density[0]) +
            (cell_index_minus[1] + d_num_subghosts_density[1])*d_subghostcell_dims_density[0] +
            (cell_index_minus[2] + d_num_subghosts_density[2])*d_subghostcell_dims_density[0]*
                d_subghostcell_dims_density[1];
        
        const int idx_density_plus = (cell_index_plus[0] + d_num_subghosts_density[0]) +
            (cell_index_plus[1] + d_num_subghosts_density[1])*d_subghostcell_dims_density[0] +
            (cell_index_plus[2] + d_num_subghosts_density[2])*d_subghostcell_dims_density[0]*
                d_subghostcell_dims_density[1];
        
        const int idx_sound_speed_minus = (cell_index_minus[0] + d_num_subghosts_sound_speed[0]) +
            (cell_index_minus[1] + d_num_subghosts_sound_speed[1])*d_subghostcell_dims_sound_speed[0] +
            (cell_index_minus[2] + d_num_subghosts_sound_speed[2])*d_subghostcell_dims_sound_speed[0]*
                d_subghostcell_dims_sound_speed[1];
        
        const int idx_sound_speed_plus = (cell_index_plus[0] + d_num_subghosts_sound_speed[0]) +
            (cell_index_plus[1] + d_num_subghosts_sound_speed[1])*d_subghostcell_dims_sound_speed[0] +
            (cell_index_plus[2] + d_num_subghosts_sound_speed[2])*d_subghostcell_dims_sound_speed[0]*
                d_subghostcell_dims_sound_speed[1];
        
        // Compute the average values.
        double rho_average, c_average;
        std::vector<double> rho_Y_average;
        rho_Y_average.reserve(d_num_species);
        switch (d_proj_mat_primitive_var_averaging)
        {
            case AVERAGING::SIMPLE:
            {
                rho_average = 0.5*(rho[idx_density_minus] + rho[idx_density_plus]);
                c_average = 0.5*(c[idx_sound_speed_minus] + c[idx_sound_speed_plus]);
                
                for (int si = 0; si < d_num_species; si++)
                {
                    rho_Y_average.push_back(0.5*(rho_Y[si][idx_minus] +
                        rho_Y[si][idx_plus]));
                }
                
                break;
            }
            case AVERAGING::ROE:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixOfPrimitiveVariables()\n"
                    << "Roe averaging is not yet implemented."
                    << std::endl);
                
                rho_average = 0.0;
                c_average   = 0.0;
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixOfPrimitiveVariables()\n"
                    << "Unknown d_proj_mat_primitive_var_averaging given."
                    << std::endl);
                
                rho_average = 0.0;
                c_average   = 0.0;
            }
        }
        
        switch (direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                projection_matrix[0][d_num_species] = 1.0;
                projection_matrix[0][d_num_species + 3] = -1.0/(rho_average*c_average);
                
                for (int si = 0; si < d_num_species; si++)
                {
                    projection_matrix[1 + si][si] = 1.0;
                    projection_matrix[1 + si][d_num_species + 3] = -rho_Y_average[si]/
                        (rho_average*c_average*c_average);
                }
                
                projection_matrix[d_num_species + 1][d_num_species + 1] = 1.0;
                
                projection_matrix[d_num_species + 2][d_num_species + 2] = 1.0;
                
                projection_matrix[d_num_species + 3][d_num_species] = 1.0;
                projection_matrix[d_num_species + 3][d_num_species + 3] = 1.0/(rho_average*c_average);
                
                break;
            }
            case DIRECTION::Y_DIRECTION:
            {
                projection_matrix[0][d_num_species + 1] = 1.0;
                projection_matrix[0][d_num_species + 3] = -1.0/(rho_average*c_average);
                
                for (int si = 0; si < d_num_species; si++)
                {
                    projection_matrix[1 + si][si] = 1.0;
                    projection_matrix[1 + si][d_num_species + 3] = -rho_Y_average[si]/
                        (rho_average*c_average*c_average);
                }
                
                projection_matrix[d_num_species + 1][d_num_species] = 1.0;
                
                projection_matrix[d_num_species + 2][d_num_species + 2] = 1.0;
                
                projection_matrix[d_num_species + 3][d_num_species + 1] = 1.0;
                projection_matrix[d_num_species + 3][d_num_species + 3] = 1.0/(rho_average*c_average);
                
                break;
            }
            case DIRECTION::Z_DIRECTION:
            {
                projection_matrix[0][d_num_species + 2] = 1.0;
                projection_matrix[0][d_num_species + 3] = -1.0/(rho_average*c_average);
                
                for (int si = 0; si < d_num_species; si++)
                {
                    projection_matrix[1 + si][si] = 1.0;
                    projection_matrix[1 + si][d_num_species + 3] = -rho_Y_average[si]/
                        (rho_average*c_average*c_average);
                }
                
                projection_matrix[d_num_species + 1][d_num_species] = 1.0;
                
                projection_matrix[d_num_species + 2][d_num_species + 1] = 1.0;
                
                projection_matrix[d_num_species + 3][d_num_species + 2] = 1.0;
                projection_matrix[d_num_species + 3][d_num_species + 3] = 1.0/(rho_average*c_average);
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixOfPrimitiveVariables()\n"
                << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                << std::endl);
            }
        }
    }
}


/*
 * Compute the local face data of inverse of projection matrix of primitive variables
 * in the registered patch.
 */
void
FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixInverseOfPrimitiveVariables(
    boost::multi_array<double, 2>& projection_matrix_inv,
    const hier::Index& cell_index_minus,
    const hier::Index& cell_index_plus,
    const DIRECTION::TYPE& direction)
{
    if (!d_proj_mat_primitive_var_registered)
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixInverseOfPrimitiveVariables()\n"
            << "Projection matrices is not yet registered."
            << std::endl);
    }
    
    projection_matrix_inv.resize(boost::extents[d_num_eqn][d_num_eqn]);
    
    // Get the cell data of the variable partial density.
    boost::shared_ptr<pdat::CellData<double> > data_partial_density =
        getGlobalCellDataPartialDensity();
    
    // Get the pointers to the cell data of partial density, total density and sound speed.
    std::vector<double*> rho_Y;
    rho_Y.reserve(d_num_species);
    for (int si = 0; si < d_num_species; si++)
    {
        rho_Y.push_back(data_partial_density->getPointer(si));
    }
    if (!d_data_sound_speed)
    {
        computeGlobalCellDataSoundSpeedWithDensityMassFractionAndPressure();
    }
    double* rho = d_data_density->getPointer(0);
    double* c = d_data_sound_speed->getPointer(0);
    
    /*
     * Fill the inverse of projection matrix with zeros.
     */
    for (int ei = 0; ei < d_num_eqn; ei++)
    {
        for (int ej = 0; ej < d_num_eqn; ej++)
        {
            projection_matrix_inv[ei][ej] = 0.0;
        }
    }
    
    // Compute the projection matrix.
    if (d_dim == tbox::Dimension(1))
    {
        // Compute the linear indices.
        const int idx_minus = cell_index_minus[0] + d_num_ghosts[0];
        const int idx_plus = cell_index_plus[0] + d_num_ghosts[0];
        const int idx_density_minus = cell_index_minus[0] + d_num_subghosts_density[0];
        const int idx_density_plus = cell_index_plus[0] + d_num_subghosts_density[0];
        const int idx_sound_speed_minus = cell_index_minus[0] + d_num_subghosts_sound_speed[0];
        const int idx_sound_speed_plus = cell_index_plus[0] + d_num_subghosts_sound_speed[0];
        
        // Compute the average values.
        double rho_average, c_average;
        std::vector<double> rho_Y_average;
        rho_Y_average.reserve(d_num_species);
        switch (d_proj_mat_primitive_var_averaging)
        {
            case AVERAGING::SIMPLE:
            {
                rho_average = 0.5*(rho[idx_density_minus] + rho[idx_density_plus]);
                c_average = 0.5*(c[idx_sound_speed_minus] + c[idx_sound_speed_plus]);
                
                for (int si = 0; si < d_num_species; si++)
                {
                    rho_Y_average.push_back(0.5*(rho_Y[si][idx_minus] +
                        rho_Y[si][idx_plus]));
                }
                
                break;
            }
            case AVERAGING::ROE:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixInverseOfPrimitiveVariables()\n"
                    << "Roe averaging is not yet implemented."
                    << std::endl);
                
                rho_average = 0.0;
                c_average   = 0.0;
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixInverseOfPrimitiveVariables()\n"
                    << "Unknown d_proj_mat_primitive_var_averaging given."
                    << std::endl);
                
                rho_average = 0.0;
                c_average   = 0.0;
            }
        }
        
        switch (direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                for (int si = 0; si < d_num_species; si++)
                {
                    projection_matrix_inv[si][0] = -0.5*rho_Y_average[si]/c_average;
                    projection_matrix_inv[si][si + 1] = 1.0;
                    projection_matrix_inv[si][d_num_eqn - 1] = 0.5*rho_Y_average[si]/c_average;
                }
                
                projection_matrix_inv[d_num_species][0] = 0.5;
                projection_matrix_inv[d_num_species][d_num_eqn - 1] = 0.5;
                
                projection_matrix_inv[d_num_species + 1][0] = -0.5*rho_average*c_average;
                projection_matrix_inv[d_num_species + 1][d_num_eqn - 1] = 0.5*rho_average*c_average;
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixInverseOfPrimitiveVariables()\n"
                << "There is only x-direction for one-dimensional problem."
                << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(2))
    {
        // Compute the linear indices.
        const int idx_minus = (cell_index_minus[0] + d_num_ghosts[0]) +
            (cell_index_minus[1] + d_num_ghosts[1])*d_ghostcell_dims[0];
        
        const int idx_plus = (cell_index_plus[0] + d_num_ghosts[0]) +
            (cell_index_plus[1] + d_num_ghosts[1])*d_ghostcell_dims[0];
        
        const int idx_density_minus = (cell_index_minus[0] + d_num_subghosts_density[0]) +
            (cell_index_minus[1] + d_num_subghosts_density[1])*d_subghostcell_dims_density[0];
        
        const int idx_density_plus = (cell_index_plus[0] + d_num_subghosts_density[0]) +
            (cell_index_plus[1] + d_num_subghosts_density[1])*d_subghostcell_dims_density[0];
        
        const int idx_sound_speed_minus = (cell_index_minus[0] + d_num_subghosts_sound_speed[0]) +
            (cell_index_minus[1] + d_num_subghosts_sound_speed[1])*d_subghostcell_dims_sound_speed[0];
        
        const int idx_sound_speed_plus = (cell_index_plus[0] + d_num_subghosts_sound_speed[0]) +
            (cell_index_plus[1] + d_num_subghosts_sound_speed[1])*d_subghostcell_dims_sound_speed[0];
        
        // Compute the average values.
        double rho_average, c_average;
        std::vector<double> rho_Y_average;
        rho_Y_average.reserve(d_num_species);
        switch (d_proj_mat_primitive_var_averaging)
        {
            case AVERAGING::SIMPLE:
            {
                rho_average = 0.5*(rho[idx_density_minus] + rho[idx_density_plus]);
                c_average = 0.5*(c[idx_sound_speed_minus] + c[idx_sound_speed_plus]);
                
                for (int si = 0; si < d_num_species; si++)
                {
                    rho_Y_average.push_back(0.5*(rho_Y[si][idx_minus] +
                        rho_Y[si][idx_plus]));
                }
                
                break;
            }
            case AVERAGING::ROE:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixInverseOfPrimitiveVariables()\n"
                    << "Roe averaging is not yet implemented."
                    << std::endl);
                
                rho_average = 0.0;
                c_average   = 0.0;
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixInverseOfPrimitiveVariables()\n"
                    << "Unknown d_proj_mat_primitive_var_averaging given."
                    << std::endl);
                
                rho_average = 0.0;
                c_average   = 0.0;
            }
        }
        
        switch (direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                for (int si = 0; si < d_num_species; si++)
                {
                    projection_matrix_inv[si][0] = -0.5*rho_Y_average[si]/c_average;
                    projection_matrix_inv[si][si + 1] = 1.0;
                    projection_matrix_inv[si][d_num_eqn - 1] = 0.5*rho_Y_average[si]/c_average;
                }
                
                projection_matrix_inv[d_num_species][0] = 0.5;
                projection_matrix_inv[d_num_species][d_num_eqn - 1] = 0.5;
                
                projection_matrix_inv[d_num_species + 1][d_num_species + 1] = 1.0;
                
                projection_matrix_inv[d_num_species + 2][0] = -0.5*rho_average*c_average;
                projection_matrix_inv[d_num_species + 2][d_num_eqn - 1] = 0.5*rho_average*c_average;
                
                break;
            }
            case DIRECTION::Y_DIRECTION:
            {
                for (int si = 0; si < d_num_species; si++)
                {
                    projection_matrix_inv[si][0] = -0.5*rho_Y_average[si]/c_average;
                    projection_matrix_inv[si][si + 1] = 1.0;
                    projection_matrix_inv[si][d_num_eqn - 1] = 0.5*rho_Y_average[si]/c_average;
                }
                
                projection_matrix_inv[d_num_species][d_num_species + 1] = 1.0;
                
                projection_matrix_inv[d_num_species + 1][0] = 0.5;
                projection_matrix_inv[d_num_species + 1][d_num_eqn - 1] = 0.5;
                
                projection_matrix_inv[d_num_species + 2][0] = -0.5*rho_average*c_average;
                projection_matrix_inv[d_num_species + 2][d_num_eqn - 1] = 0.5*rho_average*c_average;
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixInverseOfPrimitiveVariables()\n"
                << "There are only x-direction and y-direction for two-dimensional problem."
                << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(3))
    {
        const int idx_minus = (cell_index_minus[0] + d_num_ghosts[0]) +
            (cell_index_minus[1] + d_num_ghosts[1])*d_ghostcell_dims[0] +
            (cell_index_minus[2] + d_num_ghosts[2])*d_ghostcell_dims[0]*
                d_ghostcell_dims[1];
        
        const int idx_plus = (cell_index_plus[0] + d_num_ghosts[0]) +
            (cell_index_plus[1] + d_num_ghosts[1])*d_ghostcell_dims[0] +
            (cell_index_plus[2] + d_num_ghosts[2])*d_ghostcell_dims[0]*
                d_ghostcell_dims[1];
        
        const int idx_density_minus = (cell_index_minus[0] + d_num_subghosts_density[0]) +
            (cell_index_minus[1] + d_num_subghosts_density[1])*d_subghostcell_dims_density[0] +
            (cell_index_minus[2] + d_num_subghosts_density[2])*d_subghostcell_dims_density[0]*
                d_subghostcell_dims_density[1];
        
        const int idx_density_plus = (cell_index_plus[0] + d_num_subghosts_density[0]) +
            (cell_index_plus[1] + d_num_subghosts_density[1])*d_subghostcell_dims_density[0] +
            (cell_index_plus[2] + d_num_subghosts_density[2])*d_subghostcell_dims_density[0]*
                d_subghostcell_dims_density[1];
        
        const int idx_sound_speed_minus = (cell_index_minus[0] + d_num_subghosts_sound_speed[0]) +
            (cell_index_minus[1] + d_num_subghosts_sound_speed[1])*d_subghostcell_dims_sound_speed[0] +
            (cell_index_minus[2] + d_num_subghosts_sound_speed[2])*d_subghostcell_dims_sound_speed[0]*
                d_subghostcell_dims_sound_speed[1];
        
        const int idx_sound_speed_plus = (cell_index_plus[0] + d_num_subghosts_sound_speed[0]) +
            (cell_index_plus[1] + d_num_subghosts_sound_speed[1])*d_subghostcell_dims_sound_speed[0] +
            (cell_index_plus[2] + d_num_subghosts_sound_speed[2])*d_subghostcell_dims_sound_speed[0]*
                d_subghostcell_dims_sound_speed[1];
        
        // Compute the average values.
        double rho_average, c_average;
        std::vector<double> rho_Y_average;
        rho_Y_average.reserve(d_num_species);
        switch (d_proj_mat_primitive_var_averaging)
        {
            case AVERAGING::SIMPLE:
            {
                rho_average = 0.5*(rho[idx_density_minus] + rho[idx_density_plus]);
                c_average = 0.5*(c[idx_sound_speed_minus] + c[idx_sound_speed_plus]);
                
                for (int si = 0; si < d_num_species; si++)
                {
                    rho_Y_average.push_back(0.5*(rho_Y[si][idx_minus] +
                        rho_Y[si][idx_plus]));
                }
                
                break;
            }
            case AVERAGING::ROE:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixInverseOfPrimitiveVariables()\n"
                    << "Roe averaging is not yet implemented."
                    << std::endl);
                
                rho_average = 0.0;
                c_average   = 0.0;
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixInverseOfPrimitiveVariables()\n"
                    << "Unknown d_proj_mat_primitive_var_averaging given."
                    << std::endl);
                
                rho_average = 0.0;
                c_average   = 0.0;
            }
        }
        
        switch (direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                for (int si = 0; si < d_num_species; si++)
                {
                    projection_matrix_inv[si][0] = -0.5*rho_Y_average[si]/c_average;
                    projection_matrix_inv[si][si + 1] = 1.0;
                    projection_matrix_inv[si][d_num_eqn - 1] = 0.5*rho_Y_average[si]/c_average;
                }
                
                projection_matrix_inv[d_num_species][0] = 0.5;
                projection_matrix_inv[d_num_species][d_num_eqn - 1] = 0.5;
                
                projection_matrix_inv[d_num_species + 1][d_num_species + 1] = 1.0;
                
                projection_matrix_inv[d_num_species + 2][d_num_species + 2] = 1.0;
                
                projection_matrix_inv[d_num_species + 3][0] = -0.5*rho_average*c_average;
                projection_matrix_inv[d_num_species + 3][d_num_eqn - 1] = 0.5*rho_average*c_average;
                
                break;
            }
            case DIRECTION::Y_DIRECTION:
            {
                for (int si = 0; si < d_num_species; si++)
                {
                    projection_matrix_inv[si][0] = -0.5*rho_Y_average[si]/c_average;
                    projection_matrix_inv[si][si + 1] = 1.0;
                    projection_matrix_inv[si][d_num_eqn - 1] = 0.5*rho_Y_average[si]/c_average;
                }
                
                projection_matrix_inv[d_num_species][d_num_species + 1] = 1.0;
                
                projection_matrix_inv[d_num_species + 1][0] = 0.5;
                projection_matrix_inv[d_num_species + 1][d_num_eqn - 1] = 0.5;
                
                projection_matrix_inv[d_num_species + 2][d_num_species + 2] = 1.0;
                
                projection_matrix_inv[d_num_species + 3][0] = -0.5*rho_average*c_average;
                projection_matrix_inv[d_num_species + 3][d_num_eqn - 1] = 0.5*rho_average*c_average;
                
                break;
            }
            case DIRECTION::Z_DIRECTION:
            {
                for (int si = 0; si < d_num_species; si++)
                {
                    projection_matrix_inv[si][0] = -0.5*rho_Y_average[si]/c_average;
                    projection_matrix_inv[si][si + 1] = 1.0;
                    projection_matrix_inv[si][d_num_eqn - 1] = 0.5*rho_Y_average[si]/c_average;
                }
                
                projection_matrix_inv[d_num_species][d_num_species + 1] = 1.0;
                
                projection_matrix_inv[d_num_species + 1][d_num_species + 2] = 1.0;
                
                projection_matrix_inv[d_num_species + 2][0] = 0.5;
                projection_matrix_inv[d_num_species + 2][d_num_eqn - 1] = 0.5;
                
                projection_matrix_inv[d_num_species + 3][0] = -0.5*rho_average*c_average;
                projection_matrix_inv[d_num_species + 3][d_num_eqn - 1] = 0.5*rho_average*c_average;
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::computeLocalFaceProjectionMatrixInverseOfPrimitiveVariables()\n"
                << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                << std::endl);
            }
        }
    }
}


/*
 * Compute the local intercell quantities with conservative variables on each side of the face
 * from Riemann solver at face.
 * flux_face: Convective flux at face.
 * velocity_face: Velocity at face.
 */
void
FlowModelFourEqnConservative::computeLocalFaceFluxAndVelocityFromRiemannSolverWithConservativeVariables(
    std::vector<boost::reference_wrapper<double> >& flux_face,
    std::vector<boost::reference_wrapper<double> >& velocity_face,
    const std::vector<boost::reference_wrapper<double> >& conservative_variables_minus,
    const std::vector<boost::reference_wrapper<double> >& conservative_variables_plus,
    const DIRECTION::TYPE& direction,
    const RIEMANN_SOLVER::TYPE& Riemann_solver)
{
    switch (Riemann_solver)
    {
        case RIEMANN_SOLVER::HLLC:
        {
            d_Riemann_solver_HLLC->computeIntercellFluxFromConservativeVariables(
                flux_face,
                conservative_variables_minus,
                conservative_variables_plus,
                direction);
            
            break;
        }
        case RIEMANN_SOLVER::HLLC_HLL:
        {
            d_Riemann_solver_HLLC_HLL->computeIntercellFluxFromConservativeVariables(
                flux_face,
                conservative_variables_minus,
                conservative_variables_plus,
                direction);
            
            break;
        }
        default:
        {
            TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::"
            << "computeLocalFaceFluxAndVelocityFromRiemannSolverWithConservativeVariables()\n"
            << "Unknown Riemann solver required."
            << std::endl);
        }
    }
}


/*
 * Compute the local intercell quantities with primitive variables on each side of the face
 * from Riemann solver at face.
 * flux_face: Convective flux at face.
 * velocity_face: Velocity at face.
 */
void
FlowModelFourEqnConservative::computeLocalFaceFluxAndVelocityFromRiemannSolverWithPrimitiveVariables(
    std::vector<boost::reference_wrapper<double> >& flux_face,
    std::vector<boost::reference_wrapper<double> >& velocity_face,
    const std::vector<boost::reference_wrapper<double> >& primitive_variables_minus,
    const std::vector<boost::reference_wrapper<double> >& primitive_variables_plus,
    const DIRECTION::TYPE& direction,
    const RIEMANN_SOLVER::TYPE& Riemann_solver)
{
    switch (Riemann_solver)
    {
        case RIEMANN_SOLVER::HLLC:
        {
            d_Riemann_solver_HLLC->computeIntercellFluxFromPrimitiveVariables(
                flux_face,
                primitive_variables_minus,
                primitive_variables_plus,
                direction);
            
            break;
        }
        case RIEMANN_SOLVER::HLLC_HLL:
        {
            d_Riemann_solver_HLLC_HLL->computeIntercellFluxFromPrimitiveVariables(
                flux_face,
                primitive_variables_minus,
                primitive_variables_plus,
                direction);
            
            break;
        }
        default:
        {
            TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::"
            << "computeLocalFaceFluxAndVelocityFromRiemannSolverWithPrimitiveVariables()\n"
            << "Unknown Riemann solver required."
            << std::endl);
        }
    }
}


/*
 * Check whether the given conservative variables are within the bounds.
 */
bool
FlowModelFourEqnConservative::haveConservativeVariablesBounded(const std::vector<double>& conservative_variables)
{
#ifdef DEBUG_CHECK_DEV_ASSERTIONS
    TBOX_ASSERT(static_cast<int>(conservative_variables.size()) == d_num_eqn);
#endif
    
    bool are_bounded = true;
    
    // Check if the total energy is bounded.
    if (conservative_variables[d_num_species + d_dim.getValue()] < 0)
    {
        are_bounded = false;
    }
    
    // Check if the total density is bounded.
    double rho = 0.0;
    for (int si = 0; si < d_num_species; si++)
    {
        rho += conservative_variables[si];
    }
    if (rho < 0.0)
    {
        are_bounded = false;
    }
    
    // Check if the mass fractions are bounded.
    for (int si = 0; si < d_num_species; si++)
    {
        const double Y = conservative_variables[si]/rho;
        
        if (Y < d_Y_bound_lo || Y > d_Y_bound_up)
        {
            are_bounded = false;
        }
    }
    
    return are_bounded;
}


/*
 * Check whether the given primitive variables are within the bounds.
 */
bool
FlowModelFourEqnConservative::havePrimitiveVariablesBounded(const std::vector<double>& primitive_variables)
{
#ifdef DEBUG_CHECK_DEV_ASSERTIONS
    TBOX_ASSERT(static_cast<int>(primitive_variables.size()) == d_num_eqn);
#endif
    
    bool are_bounded = true;
    
    // Check if the pressure is bounded.
    if (primitive_variables[d_num_species + d_dim.getValue()] < 0)
    {
        are_bounded = false;
    }
    
    // Check if the total density is bounded.
    double rho = 0.0;
    for (int si = 0; si < d_num_species; si++)
    {
        rho += primitive_variables[si];
    }
    if (rho < 0.0)
    {
        are_bounded = false;
    }
    
    // Check if the mass fractions are bounded.
    for (int si = 0; si < d_num_species; si++)
    {
        const double Y = primitive_variables[si]/rho;
        
        if (Y < d_Y_bound_lo || Y > d_Y_bound_up)
        {
            are_bounded = false;
        }
    }
    
    return are_bounded;
}


/*
 * Convert vector of pointers of conservative cell data to vectors of pointers of primitive cell data.
 */
void
FlowModelFourEqnConservative::convertLocalCellDataPointersConservativeVariablesToPrimitiveVariables(
    const std::vector<const double*>& conservative_variables,
    const std::vector<double*>& primitive_variables)
{
    const std::vector<const double*>& Q = conservative_variables;
    const std::vector<double*>&       V = primitive_variables;
    
    // Compute the mixture density.
    std::vector<const double*> rho_Y_ptr;
    rho_Y_ptr.reserve(d_num_species);
    for (int si = 0; si < d_num_species; si++)
    {
        rho_Y_ptr.push_back(Q[si]);
    }
    const double rho = d_equation_of_state_mixing_rules->getMixtureDensity(
        rho_Y_ptr);
    
    /*
     * Compute the internal energy.
     */
    double epsilon = 0.0;
    if (d_dim == tbox::Dimension(1))
    {
        epsilon = ((*Q[d_num_species + d_dim.getValue()]) -
            0.5*((*Q[d_num_species])*(*Q[d_num_species]))/rho)/rho;
    }
    else if (d_dim == tbox::Dimension(2))
    {
        epsilon = ((*Q[d_num_species + d_dim.getValue()]) -
            0.5*((*Q[d_num_species])*(*Q[d_num_species]) +
            (*Q[d_num_species + 1])*(*Q[d_num_species + 1]))/rho)/rho;
    }
    else if (d_dim == tbox::Dimension(3))
    {
        epsilon = ((*Q[d_num_species + d_dim.getValue()]) -
            0.5*((*Q[d_num_species])*(*Q[d_num_species]) +
            (*Q[d_num_species + 1])*(*Q[d_num_species + 1]) +
            (*Q[d_num_species + 2])*(*Q[d_num_species + 2]))/rho)/rho;
    }
    
    /*
     * Compute the mass fractions.
     */
    double Y[d_num_species];
    for (int si = 0; si < d_num_species; si++)
    {
        Y[si] = (*Q[si])/rho;
    }
    
    /*
     * Get the pointers to the mass fractions.
     */
    std::vector<const double*> Y_ptr;
    Y_ptr.reserve(d_num_species);
    for (int si = 0; si < d_num_species; si++)
    {
        Y_ptr.push_back(&Y[si]);
    }
    
    // Compute the pressure.
    const double p = d_equation_of_state_mixing_rules->getPressure(
        &rho,
        &epsilon,
        Y_ptr);
    
    // Convert the conservative variables to primitive variables.
    for (int si = 0; si < d_num_species; si++)
    {
        *V[si] = *Q[si];
    }
    for (int di = 0; di < d_dim.getValue(); di++)
    {
        *V[d_num_species + di] = (*Q[d_num_species + di])/rho;
    }
    *V[d_num_species + d_dim.getValue()] = p;
}


/*
 * Convert vector of pointers of primitive cell data to vectors of pointers of conservative cell data.
 */
void
FlowModelFourEqnConservative::convertLocalCellDataPointersPrimitiveVariablesToConservativeVariables(
    const std::vector<const double*>& primitive_variables,
    const std::vector<double*>& conservative_variables)
{
    const std::vector<const double*>& V = primitive_variables;
    const std::vector<double*>&       Q = conservative_variables;
    
    // Compute the mixture density.
    std::vector<const double*> rho_Y_ptr;
    rho_Y_ptr.reserve(d_num_species);
    for (int si = 0; si < d_num_species; si++)
    {
        rho_Y_ptr.push_back(V[si]);
    }
    const double rho = d_equation_of_state_mixing_rules->getMixtureDensity(
        rho_Y_ptr);
    
    /*
     * Compute the mass fractions.
     */
    double Y[d_num_species];
    for (int si = 0; si < d_num_species; si++)
    {
        Y[si] = (*V[si])/rho;
    }
    
    /*
     * Get the pointers to the mass fractions.
     */
    std::vector<const double*> Y_ptr;
    Y_ptr.reserve(d_num_species);
    for (int si = 0; si < d_num_species; si++)
    {
        Y_ptr.push_back(&Y[si]);
    }
    
    // Compute the total energy.
    const double epsilon = d_equation_of_state_mixing_rules->getInternalEnergy(
        &rho,
        V[d_num_species + d_dim.getValue()],
        Y_ptr);
    
    const double E = epsilon + 0.5*rho*((*Q[d_num_species])*(*Q[d_num_species]) +
        (*Q[d_num_species + 1])*(*Q[d_num_species + 1]) +
        (*Q[d_num_species + 2])*(*Q[d_num_species + 2]));
    
    // Convert the primitive variables to conservative variables.
    for (int si = 0; si < d_num_species; si++)
    {
        *Q[si] = *V[si];
    }
    for (int di = 0; di < d_dim.getValue(); di++)
    {
        *Q[d_num_species + di] = rho*(*V[d_num_species + di]);
    }
    *Q[d_num_species + d_dim.getValue()] = E;
}


/*
 * Get the variables for the derivatives in the diffusive fluxes.
 */
void
FlowModelFourEqnConservative::getDiffusiveFluxVariablesForDerivative(
    std::vector<std::vector<boost::shared_ptr<pdat::CellData<double> > > >& derivative_var_data,
    std::vector<std::vector<int> >& derivative_var_component_idx,
    const DIRECTION::TYPE& flux_direction,
    const DIRECTION::TYPE& derivative_direction)
{
    derivative_var_data.resize(d_num_eqn);
    derivative_var_component_idx.resize(d_num_eqn);
    
    if (!d_data_mass_fraction)
    {
        computeGlobalCellDataMassFractionWithDensity();
    }
    
    if (!d_data_velocity)
    {
        computeGlobalCellDataVelocityWithDensity();
    }
    
    if (!d_data_temperature)
    {
        computeGlobalCellDataTemperatureWithDensityMassFractionAndPressure();
    }
    
    if (d_dim == tbox::Dimension(1))
    {
        switch (flux_direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(d_num_species + 1);
                            derivative_var_component_idx[si].resize(d_num_species + 1);
                            
                            derivative_var_data[si][0] = d_data_mass_fraction;
                            derivative_var_component_idx[si][0] = si;
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                derivative_var_data[si][1 + sj] = d_data_mass_fraction;
                                derivative_var_component_idx[si][1 + sj] = sj;
                            }
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 0;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 1].resize(
                            2 + d_num_species*(d_num_species + 1));
                        derivative_var_component_idx[d_num_species + 1].resize(
                            2 + d_num_species*(d_num_species + 1));
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 1][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 0;
                        
                        // Variable T.
                        derivative_var_data[d_num_species + 1][1] = d_data_temperature;
                        derivative_var_component_idx[d_num_species + 1][1] = 0;
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[d_num_species + 1][2 + si*(d_num_species + 1)] =
                                d_data_mass_fraction;
                            derivative_var_component_idx[d_num_species + 1][2 + si*(d_num_species + 1)] =
                                si;
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                derivative_var_data[d_num_species + 1][3 + si*(d_num_species + 1) + sj] =
                                    d_data_mass_fraction;
                                derivative_var_component_idx[d_num_species + 1][3 + si*(d_num_species + 1) + sj] =
                                    sj;
                            }
                        }
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelFourEqnConservative::getDiffusiveFluxVariablesForDerivative()\n"
                            << "There are only x-direction for one-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::getDiffusiveFluxVariablesForDerivative()\n"
                    << "There are only x-direction for one-dimensional problem."
                    << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(2))
    {
        switch (flux_direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(d_num_species + 1);
                            derivative_var_component_idx[si].resize(d_num_species + 1);
                            
                            derivative_var_data[si][0] = d_data_mass_fraction;
                            derivative_var_component_idx[si][0] = si;
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                derivative_var_data[si][1 + sj] = d_data_mass_fraction;
                                derivative_var_component_idx[si][1 + sj] = sj;
                            }
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 0;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 1][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 1;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 2].resize(
                            3 + d_num_species*(d_num_species + 1));
                        derivative_var_component_idx[d_num_species + 2].resize(
                            3 + d_num_species*(d_num_species + 1));
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 2][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 2][1] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 2][1] = 1;
                        
                        // Variable T.
                        derivative_var_data[d_num_species + 2][2] = d_data_temperature;
                        derivative_var_component_idx[d_num_species + 2][2] = 0;
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[d_num_species + 2][3 + si*(d_num_species + 1)] =
                                d_data_mass_fraction;
                            derivative_var_component_idx[d_num_species + 2][3 + si*(d_num_species + 1)] =
                                si;
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                derivative_var_data[d_num_species + 2][4 + si*(d_num_species + 1) + sj] =
                                    d_data_mass_fraction;
                                derivative_var_component_idx[d_num_species + 2][4 + si*(d_num_species + 1) + sj] =
                                    sj;
                            }
                        }
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 1;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 1][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 0;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 2].resize(2);
                        derivative_var_component_idx[d_num_species + 2].resize(2);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 2][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 2][1] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 2][1] = 1;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelFourEqnConservative::getDiffusiveFluxVariablesForDerivative()\n"
                            << "There are only x-direction and y-direction for two-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            case DIRECTION::Y_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 1;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 1][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 0;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 2].resize(2);
                        derivative_var_component_idx[d_num_species + 2].resize(2);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 2][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 2][1] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 2][1] = 1;
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(d_num_species + 1);
                            derivative_var_component_idx[si].resize(d_num_species + 1);
                            
                            derivative_var_data[si][0] = d_data_mass_fraction;
                            derivative_var_component_idx[si][0] = si;
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                derivative_var_data[si][1 + sj] = d_data_mass_fraction;
                                derivative_var_component_idx[si][1 + sj] = sj;
                            }
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 0;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 1][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 1;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 2].resize(
                            3 + d_num_species*(d_num_species + 1));
                        derivative_var_component_idx[d_num_species + 2].resize(
                            3 + d_num_species*(d_num_species + 1));
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 2][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 2][1] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 2][1] = 1;
                        
                        // Variable T.
                        derivative_var_data[d_num_species + 2][2] = d_data_temperature;
                        derivative_var_component_idx[d_num_species + 2][2] = 0;
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[d_num_species + 2][3 + si*(d_num_species + 1)] =
                                d_data_mass_fraction;
                            derivative_var_component_idx[d_num_species + 2][3 + si*(d_num_species + 1)] =
                                si;
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                derivative_var_data[d_num_species + 2][4 + si*(d_num_species + 1) + sj] =
                                    d_data_mass_fraction;
                                derivative_var_component_idx[d_num_species + 2][4 + si*(d_num_species + 1) + sj] =
                                    sj;
                            }
                        }
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelFourEqnConservative::getDiffusiveFluxVariablesForDerivative()\n"
                            << "There are only x-direction and y-direction for two-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::getDiffusiveFluxVariablesForDerivative()\n"
                    << "There are only x-direction and y-direction for two-dimensional problem."
                    << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(3))
    {
        switch (flux_direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(d_num_species + 1);
                            derivative_var_component_idx[si].resize(d_num_species + 1);
                            
                            derivative_var_data[si][0] = d_data_mass_fraction;
                            derivative_var_component_idx[si][0] = si;
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                derivative_var_data[si][1 + sj] = d_data_mass_fraction;
                                derivative_var_component_idx[si][1 + sj] = sj;
                            }
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 0;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 1][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 1;
                        
                        derivative_var_data[d_num_species + 2].resize(1);
                        derivative_var_component_idx[d_num_species + 2].resize(1);
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 2][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(
                            4 + d_num_species*(d_num_species + 1));
                        derivative_var_component_idx[d_num_species + 3].resize(
                            4 + d_num_species*(d_num_species + 1));
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 3][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 3][1] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 1;
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 3][2] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][2] = 2;
                        
                        // Variable T.
                        derivative_var_data[d_num_species + 3][3] = d_data_temperature;
                        derivative_var_component_idx[d_num_species + 3][3] = 0;
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[d_num_species + 3][4 + si*(d_num_species + 1)] =
                                d_data_mass_fraction;
                            derivative_var_component_idx[d_num_species + 3][4 + si*(d_num_species + 1)] =
                                si;
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                derivative_var_data[d_num_species + 3][5 + si*(d_num_species + 1) + sj] =
                                    d_data_mass_fraction;
                                derivative_var_component_idx[d_num_species + 3][5 + si*(d_num_species + 1) + sj] =
                                    sj;
                            }
                        }
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 1;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 1][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 0;
                        
                        derivative_var_data[d_num_species + 2].resize(0);
                        derivative_var_component_idx[d_num_species + 2].resize(0);
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(2);
                        derivative_var_component_idx[d_num_species + 3].resize(2);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 3][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 3][1] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 1;
                        
                        break;
                    }
                    case DIRECTION::Z_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable w.
                        derivative_var_data[d_num_species][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 2;
                        
                        derivative_var_data[d_num_species + 1].resize(0);
                        derivative_var_component_idx[d_num_species + 1].resize(0);
                        
                        derivative_var_data[d_num_species + 2].resize(1);
                        derivative_var_component_idx[d_num_species + 2].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 2][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 0;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(2);
                        derivative_var_component_idx[d_num_species + 3].resize(2);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 3][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 0;
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 3][1] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 2;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelFourEqnConservative::getDiffusiveFluxVariablesForDerivative()\n"
                            << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            case DIRECTION::Y_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 1;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 1][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 0;
                        
                        derivative_var_data[d_num_species + 2].resize(0);
                        derivative_var_component_idx[d_num_species + 2].resize(0);
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(2);
                        derivative_var_component_idx[d_num_species + 3].resize(2);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 3][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 3][1] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 1;
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(d_num_species + 1);
                            derivative_var_component_idx[si].resize(d_num_species + 1);
                            
                            derivative_var_data[si][0] = d_data_mass_fraction;
                            derivative_var_component_idx[si][0] = si;
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                derivative_var_data[si][1 + sj] = d_data_mass_fraction;
                                derivative_var_component_idx[si][1 + sj] = sj;
                            }
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 0;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 1][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 1;
                        
                        derivative_var_data[d_num_species + 2].resize(1);
                        derivative_var_component_idx[d_num_species + 2].resize(1);
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 2][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(
                            4 + d_num_species*(d_num_species + 1));
                        derivative_var_component_idx[d_num_species + 3].resize(
                            4 + d_num_species*(d_num_species + 1));
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 3][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 3][1] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 1;
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 3][2] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][2] = 2;
                        
                        // Variable T.
                        derivative_var_data[d_num_species + 3][3] = d_data_temperature;
                        derivative_var_component_idx[d_num_species + 3][3] = 0;
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[d_num_species + 3][4 + si*(d_num_species + 1)] =
                                d_data_mass_fraction;
                            derivative_var_component_idx[d_num_species + 3][4 + si*(d_num_species + 1)] =
                                si;
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                derivative_var_data[d_num_species + 3][5 + si*(d_num_species + 1) + sj] =
                                    d_data_mass_fraction;
                                derivative_var_component_idx[d_num_species + 3][5 + si*(d_num_species + 1) + sj] =
                                    sj;
                            }
                        }
                        
                        break;
                    }
                    case DIRECTION::Z_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(0);
                        derivative_var_component_idx[d_num_species].resize(0);
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 1][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 2;
                        
                        derivative_var_data[d_num_species + 2].resize(1);
                        derivative_var_component_idx[d_num_species + 2].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 2][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 1;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(2);
                        derivative_var_component_idx[d_num_species + 3].resize(2);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 3][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 1;
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 3][1] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 2;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelFourEqnConservative::getDiffusiveFluxVariablesForDerivative()\n"
                            << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            case DIRECTION::Z_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable w.
                        derivative_var_data[d_num_species][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 2;
                        
                        derivative_var_data[d_num_species + 1].resize(0);
                        derivative_var_component_idx[d_num_species + 1].resize(0);
                        
                        derivative_var_data[d_num_species + 2].resize(1);
                        derivative_var_component_idx[d_num_species + 2].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 2][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 0;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(2);
                        derivative_var_component_idx[d_num_species + 3].resize(2);
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 3][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 0;
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 3][1] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 2;
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(0);
                            derivative_var_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(0);
                        derivative_var_component_idx[d_num_species].resize(0);
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 1][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 2;
                        
                        derivative_var_data[d_num_species + 2].resize(1);
                        derivative_var_component_idx[d_num_species + 2].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 2][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 1;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(2);
                        derivative_var_component_idx[d_num_species + 3].resize(2);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 3][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 1;
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 3][1] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 2;
                        
                        break;
                    }
                    case DIRECTION::Z_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[si].resize(d_num_species + 1);
                            derivative_var_component_idx[si].resize(d_num_species + 1);
                            
                            derivative_var_data[si][0] = d_data_mass_fraction;
                            derivative_var_component_idx[si][0] = si;
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                derivative_var_data[si][1 + sj] = d_data_mass_fraction;
                                derivative_var_component_idx[si][1 + sj] = sj;
                            }
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        derivative_var_data[d_num_species].resize(1);
                        derivative_var_component_idx[d_num_species].resize(1);
                        
                        // Variable u.
                        derivative_var_data[d_num_species][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species][0] = 0;
                        
                        derivative_var_data[d_num_species + 1].resize(1);
                        derivative_var_component_idx[d_num_species + 1].resize(1);
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 1][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 1][0] = 1;
                        
                        derivative_var_data[d_num_species + 2].resize(1);
                        derivative_var_component_idx[d_num_species + 2].resize(1);
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 2][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 2][0] = 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        derivative_var_data[d_num_species + 3].resize(
                            4 + d_num_species*(d_num_species + 1));
                        derivative_var_component_idx[d_num_species + 3].resize(
                            4 + d_num_species*(d_num_species + 1));
                        
                        // Variable u.
                        derivative_var_data[d_num_species + 3][0] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][0] = 0;
                        
                        // Variable v.
                        derivative_var_data[d_num_species + 3][1] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][1] = 1;
                        
                        // Variable w.
                        derivative_var_data[d_num_species + 3][2] = d_data_velocity;
                        derivative_var_component_idx[d_num_species + 3][2] = 2;
                        
                        // Variable T.
                        derivative_var_data[d_num_species + 3][3] = d_data_temperature;
                        derivative_var_component_idx[d_num_species + 3][3] = 0;
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            derivative_var_data[d_num_species + 3][4 + si*(d_num_species + 1)] =
                                d_data_mass_fraction;
                            derivative_var_component_idx[d_num_species + 3][4 + si*(d_num_species + 1)] =
                                si;
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                derivative_var_data[d_num_species + 3][5 + si*(d_num_species + 1) + sj] =
                                    d_data_mass_fraction;
                                derivative_var_component_idx[d_num_species + 3][5 + si*(d_num_species + 1) + sj] =
                                    sj;
                            }
                        }
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelFourEqnConservative::getDiffusiveFluxVariablesForDerivative()\n"
                            << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::getDiffusiveFluxVariablesForDerivative()\n"
                    << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                    << std::endl);
            }
        }
    }
}


/*
 * Get the diffusivities in the diffusive flux.
 */
void
FlowModelFourEqnConservative::getDiffusiveFluxDiffusivities(
    std::vector<std::vector<boost::shared_ptr<pdat::CellData<double> > > >& diffusivities_data,
    std::vector<std::vector<int> >& diffusivities_component_idx,
    const DIRECTION::TYPE& flux_direction,
    const DIRECTION::TYPE& derivative_direction)
{
    if (!d_equation_of_mass_diffusivity_mixing_rules ||
        !d_equation_of_shear_viscosity_mixing_rules ||
        !d_equation_of_bulk_viscosity_mixing_rules ||
        !d_equation_of_thermal_conductivity_mixing_rules)
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::getDiffusiveFluxDiffusivities()\n"
            << "Either mixing rule of mass diffusivity, shear viscosity, bulk viscosity or"
            << " thermal conductivity is not initialized."
            << std::endl);
    }
    
    diffusivities_data.resize(d_num_eqn);
    diffusivities_component_idx.resize(d_num_eqn);
        
    if (!d_data_diffusivities)
    {
        if (!d_data_density)
        {
            computeGlobalCellDataDensity();
        }
        
        if (!d_data_mass_fraction)
        {
            computeGlobalCellDataMassFractionWithDensity();
        }
        
        if (!d_data_velocity)
        {
            computeGlobalCellDataVelocityWithDensity();
        }
        
        if (!d_data_pressure)
        {
            computeGlobalCellDataPressureWithDensityMassFractionAndInternalEnergy();
        }
        
        if (!d_data_temperature)
        {
            computeGlobalCellDataTemperatureWithDensityMassFractionAndPressure();
        }
        
        // Get the pointers to the cell data of density, mass fraction, pressure and temperature.
        double* rho = d_data_density->getPointer(0);
        std::vector<double*> Y;
        Y.reserve(d_num_species);
        for (int si = 0; si < d_num_species; si++)
        {
            Y.push_back(d_data_mass_fraction->getPointer(si));
        }
        double* p = d_data_pressure->getPointer(0);
        double* T = d_data_temperature->getPointer(0);
        
        /*
         * Compute mass diffusivities.
         */
        
        boost::shared_ptr<pdat::CellData<double> > data_mass_diffusivities(
            new pdat::CellData<double>(d_interior_box, d_num_species, d_num_subghosts_diffusivities));
        
        std::vector<double*> D;
        D.reserve(d_num_species);
        for (int si = 0; si < d_num_species; si++)
        {
            D.push_back(data_mass_diffusivities->getPointer(si));
        }
        
        if (d_dim == tbox::Dimension(1))
        {
            for (int i = -d_num_subghosts_diffusivities[0];
                 i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                 i++)
            {
                // Compute the linear indices.
                const int idx_diffusivities = i + d_num_subghosts_diffusivities[0];
                const int idx_pressure = i + d_num_subghosts_pressure[0];
                const int idx_temperature = i + d_num_subghosts_temperature[0];
                const int idx_mass_fraction = i + d_num_subghosts_mass_fraction[0];
                
                std::vector<double*> D_ptr;
                D_ptr.reserve(d_num_species);
                for (int si = 0; si < d_num_species; si++)
                {
                    D_ptr.push_back(&D[si][idx_diffusivities]);
                }
                
                std::vector<const double*> Y_ptr;
                Y_ptr.reserve(d_num_species);
                for (int si = 0; si < d_num_species; si++)
                {
                    Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                }
                
                d_equation_of_mass_diffusivity_mixing_rules->
                    getMassDiffusivities(
                        D_ptr,
                        &p[idx_pressure],
                        &T[idx_temperature],
                        Y_ptr);
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            for (int j = -d_num_subghosts_diffusivities[1];
                 j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                 j++)
            {
                for (int i = -d_num_subghosts_diffusivities[0];
                     i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                        (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0];
                    
                    const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                        (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0];
                    
                    const int idx_temperature = (i + d_num_subghosts_temperature[0]) +
                        (j + d_num_subghosts_temperature[1])*d_subghostcell_dims_temperature[0];
                    
                    const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                        (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0];
                    
                    std::vector<double*> D_ptr;
                    D_ptr.reserve(d_num_species);
                    for (int si = 0; si < d_num_species; si++)
                    {
                        D_ptr.push_back(&D[si][idx_diffusivities]);
                    }
                    
                    std::vector<const double*> Y_ptr;
                    Y_ptr.reserve(d_num_species);
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                    }
                    
                    d_equation_of_mass_diffusivity_mixing_rules->
                        getMassDiffusivities(
                            D_ptr,
                            &p[idx_pressure],
                            &T[idx_temperature],
                            Y_ptr);
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            for (int k = -d_num_subghosts_diffusivities[2];
                 k < d_interior_dims[2] + d_num_subghosts_diffusivities[2];
                 k++)
            {
                for (int j = -d_num_subghosts_diffusivities[1];
                     j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_diffusivities[0];
                         i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                         i++)
                    {
                        const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                            (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0] +
                            (k + d_num_subghosts_diffusivities[2])*d_subghostcell_dims_diffusivities[0]*
                                d_subghostcell_dims_diffusivities[1];
                        
                        const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                            (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0] +
                            (k + d_num_subghosts_pressure[2])*d_subghostcell_dims_pressure[0]*
                                d_subghostcell_dims_pressure[1];
                        
                        const int idx_temperature = (i + d_num_subghosts_temperature[0]) +
                            (j + d_num_subghosts_temperature[1])*d_subghostcell_dims_temperature[0] +
                            (k + d_num_subghosts_temperature[2])*d_subghostcell_dims_temperature[0]*
                                d_subghostcell_dims_temperature[1];
                        
                        const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                            (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0] +
                            (k + d_num_subghosts_mass_fraction[2])*d_subghostcell_dims_mass_fraction[0]*
                                d_subghostcell_dims_mass_fraction[1];
                        
                        std::vector<double*> D_ptr;
                        D_ptr.reserve(d_num_species);
                        for (int si = 0; si < d_num_species; si++)
                        {
                            D_ptr.push_back(&D[si][idx_diffusivities]);
                        }
                        
                        std::vector<const double*> Y_ptr;
                        Y_ptr.reserve(d_num_species);
                        for (int si = 0; si < d_num_species; si++)
                        {
                            Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                        }
                        
                        d_equation_of_mass_diffusivity_mixing_rules->
                            getMassDiffusivities(
                                D_ptr,
                                &p[idx_pressure],
                                &T[idx_temperature],
                                Y_ptr);
                    }
                }
            }
        }
        
        /*
         * Compute shear viscosity.
         */
        
        boost::shared_ptr<pdat::CellData<double> > data_shear_viscosity(
            new pdat::CellData<double>(d_interior_box, 1, d_num_subghosts_diffusivities));
        
        double* mu = data_shear_viscosity->getPointer(0);
        
        if (d_dim == tbox::Dimension(1))
        {
            for (int i = -d_num_subghosts_diffusivities[0];
                 i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                 i++)
            {
                // Compute the linear indices.
                const int idx_diffusivities = i + d_num_subghosts_diffusivities[0];
                const int idx_pressure = i + d_num_subghosts_pressure[0];
                const int idx_temperature = i + d_num_subghosts_temperature[0];
                const int idx_mass_fraction = i + d_num_subghosts_mass_fraction[0];
                
                std::vector<const double*> Y_ptr;
                Y_ptr.reserve(d_num_species);
                for (int si = 0; si < d_num_species; si++)
                {
                    Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                }
                
                mu[idx_diffusivities] = d_equation_of_shear_viscosity_mixing_rules->
                    getShearViscosity(
                        &p[idx_pressure],
                        &T[idx_temperature],
                        Y_ptr);
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            for (int j = -d_num_subghosts_diffusivities[1];
                 j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                 j++)
            {
                for (int i = -d_num_subghosts_diffusivities[0];
                     i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                        (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0];
                    
                    const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                        (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0];
                    
                    const int idx_temperature = (i + d_num_subghosts_temperature[0]) +
                        (j + d_num_subghosts_temperature[1])*d_subghostcell_dims_temperature[0];
                    
                    const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                        (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0];
                    
                    std::vector<const double*> Y_ptr;
                    Y_ptr.reserve(d_num_species);
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                    }
                    
                    mu[idx_diffusivities] = d_equation_of_shear_viscosity_mixing_rules->
                        getShearViscosity(
                            &p[idx_pressure],
                            &T[idx_temperature],
                            Y_ptr);
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            for (int k = -d_num_subghosts_diffusivities[2];
                 k < d_interior_dims[2] + d_num_subghosts_diffusivities[2];
                 k++)
            {
                for (int j = -d_num_subghosts_diffusivities[1];
                     j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_diffusivities[0];
                         i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                         i++)
                    {
                        const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                            (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0] +
                            (k + d_num_subghosts_diffusivities[2])*d_subghostcell_dims_diffusivities[0]*
                                d_subghostcell_dims_diffusivities[1];
                        
                        const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                            (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0] +
                            (k + d_num_subghosts_pressure[2])*d_subghostcell_dims_pressure[0]*
                                d_subghostcell_dims_pressure[1];
                        
                        const int idx_temperature = (i + d_num_subghosts_temperature[0]) +
                            (j + d_num_subghosts_temperature[1])*d_subghostcell_dims_temperature[0] +
                            (k + d_num_subghosts_temperature[2])*d_subghostcell_dims_temperature[0]*
                                d_subghostcell_dims_temperature[1];
                        
                        const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                            (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0] +
                            (k + d_num_subghosts_mass_fraction[2])*d_subghostcell_dims_mass_fraction[0]*
                                d_subghostcell_dims_mass_fraction[1];
                        
                        std::vector<const double*> Y_ptr;
                        Y_ptr.reserve(d_num_species);
                        for (int si = 0; si < d_num_species; si++)
                        {
                            Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                        }
                        
                        mu[idx_diffusivities] = d_equation_of_shear_viscosity_mixing_rules->
                            getShearViscosity(
                                &p[idx_pressure],
                                &T[idx_temperature],
                                Y_ptr);
                    }
                }
            }
        }
        
        /*
         * Compute bulk viscosity.
         */
        
        boost::shared_ptr<pdat::CellData<double> > data_bulk_viscosity(
            new pdat::CellData<double>(d_interior_box, 1, d_num_subghosts_diffusivities));
        
        double* mu_v = data_bulk_viscosity->getPointer(0);
        
        if (d_dim == tbox::Dimension(1))
        {
            for (int i = -d_num_subghosts_diffusivities[0];
                 i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                 i++)
            {
                // Compute the linear indices.
                const int idx_diffusivities = i + d_num_subghosts_diffusivities[0];
                const int idx_pressure = i + d_num_subghosts_pressure[0];
                const int idx_temperature = i + d_num_subghosts_temperature[0];
                const int idx_mass_fraction = i + d_num_subghosts_mass_fraction[0];
                
                std::vector<const double*> Y_ptr;
                Y_ptr.reserve(d_num_species);
                for (int si = 0; si < d_num_species; si++)
                {
                    Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                }
                
                mu_v[idx_diffusivities] = d_equation_of_bulk_viscosity_mixing_rules->
                    getBulkViscosity(
                        &p[idx_pressure],
                        &T[idx_temperature],
                        Y_ptr);
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            for (int j = -d_num_subghosts_diffusivities[1];
                 j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                 j++)
            {
                for (int i = -d_num_subghosts_diffusivities[0];
                     i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                        (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0];
                    
                    const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                        (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0];
                    
                    const int idx_temperature = (i + d_num_subghosts_temperature[0]) +
                        (j + d_num_subghosts_temperature[1])*d_subghostcell_dims_temperature[0];
                    
                    const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                        (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0];
                    
                    std::vector<const double*> Y_ptr;
                    Y_ptr.reserve(d_num_species);
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                    }
                    
                    mu_v[idx_diffusivities] = d_equation_of_bulk_viscosity_mixing_rules->
                        getBulkViscosity(
                            &p[idx_pressure],
                            &T[idx_temperature],
                            Y_ptr);
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            for (int k = -d_num_subghosts_diffusivities[2];
                 k < d_interior_dims[2] + d_num_subghosts_diffusivities[2];
                 k++)
            {
                for (int j = -d_num_subghosts_diffusivities[1];
                     j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_diffusivities[0];
                         i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                         i++)
                    {
                        const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                            (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0] +
                            (k + d_num_subghosts_diffusivities[2])*d_subghostcell_dims_diffusivities[0]*
                                d_subghostcell_dims_diffusivities[1];
                        
                        const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                            (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0] +
                            (k + d_num_subghosts_pressure[2])*d_subghostcell_dims_pressure[0]*
                                d_subghostcell_dims_pressure[1];
                        
                        const int idx_temperature = (i + d_num_subghosts_temperature[0]) +
                            (j + d_num_subghosts_temperature[1])*d_subghostcell_dims_temperature[0] +
                            (k + d_num_subghosts_temperature[2])*d_subghostcell_dims_temperature[0]*
                                d_subghostcell_dims_temperature[1];
                        
                        const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                            (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0] +
                            (k + d_num_subghosts_mass_fraction[2])*d_subghostcell_dims_mass_fraction[0]*
                                d_subghostcell_dims_mass_fraction[1];
                        
                        std::vector<const double*> Y_ptr;
                        Y_ptr.reserve(d_num_species);
                        for (int si = 0; si < d_num_species; si++)
                        {
                            Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                        }
                        
                        mu_v[idx_diffusivities] = d_equation_of_bulk_viscosity_mixing_rules->
                            getBulkViscosity(
                                &p[idx_pressure],
                                &T[idx_temperature],
                                Y_ptr);
                    }
                }
            }
        }
        
        /*
         * Compute thermal conductivity.
         */
        
        boost::shared_ptr<pdat::CellData<double> > data_thermal_conductivity(new pdat::CellData<double>(
            d_interior_box, 1, d_num_subghosts_diffusivities));
        
        double* kappa = data_thermal_conductivity->getPointer(0);
        
        if (d_dim == tbox::Dimension(1))
        {
            for (int i = -d_num_subghosts_diffusivities[0];
                 i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                 i++)
            {
                // Compute the linear indices.
                const int idx_diffusivities = i + d_num_subghosts_diffusivities[0];
                const int idx_pressure = i + d_num_subghosts_pressure[0];
                const int idx_temperature = i + d_num_subghosts_temperature[0];
                const int idx_mass_fraction = i + d_num_subghosts_mass_fraction[0];
                
                std::vector<const double*> Y_ptr;
                Y_ptr.reserve(d_num_species);
                for (int si = 0; si < d_num_species; si++)
                {
                    Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                }
                
                kappa[idx_diffusivities] = d_equation_of_thermal_conductivity_mixing_rules->
                    getThermalConductivity(
                        &p[idx_pressure],
                        &T[idx_temperature],
                        Y_ptr);
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            for (int j = -d_num_subghosts_diffusivities[1];
                 j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                 j++)
            {
                for (int i = -d_num_subghosts_diffusivities[0];
                     i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                        (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0];
                    
                    const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                        (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0];
                    
                    const int idx_temperature = (i + d_num_subghosts_temperature[0]) +
                        (j + d_num_subghosts_temperature[1])*d_subghostcell_dims_temperature[0];
                    
                    const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                        (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0];
                    
                    std::vector<const double*> Y_ptr;
                    Y_ptr.reserve(d_num_species);
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                    }
                    
                    kappa[idx_diffusivities] = d_equation_of_thermal_conductivity_mixing_rules->
                        getThermalConductivity(
                            &p[idx_pressure],
                            &T[idx_temperature],
                            Y_ptr);
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            for (int k = -d_num_subghosts_diffusivities[2];
                 k < d_interior_dims[2] + d_num_subghosts_diffusivities[2];
                 k++)
            {
                for (int j = -d_num_subghosts_diffusivities[1];
                     j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_diffusivities[0];
                         i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                         i++)
                    {
                        const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                            (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0] +
                            (k + d_num_subghosts_diffusivities[2])*d_subghostcell_dims_diffusivities[0]*
                                d_subghostcell_dims_diffusivities[1];
                        
                        const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                            (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0] +
                            (k + d_num_subghosts_pressure[2])*d_subghostcell_dims_pressure[0]*
                                d_subghostcell_dims_pressure[1];
                        
                        const int idx_temperature = (i + d_num_subghosts_temperature[0]) +
                            (j + d_num_subghosts_temperature[1])*d_subghostcell_dims_temperature[0] +
                            (k + d_num_subghosts_temperature[2])*d_subghostcell_dims_temperature[0]*
                                d_subghostcell_dims_temperature[1];
                        
                        const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                            (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0] +
                            (k + d_num_subghosts_mass_fraction[2])*d_subghostcell_dims_mass_fraction[0]*
                                d_subghostcell_dims_mass_fraction[1];
                        
                        std::vector<const double*> Y_ptr;
                        Y_ptr.reserve(d_num_species);
                        for (int si = 0; si < d_num_species; si++)
                        {
                            Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                        }
                        
                        kappa[idx_diffusivities] = d_equation_of_thermal_conductivity_mixing_rules->
                            getThermalConductivity(
                                &p[idx_pressure],
                                &T[idx_temperature],
                                Y_ptr);
                    }
                }
            }
        }
        
        /*
         * Compute enthalpy of each species.
         */
        
        std::vector<boost::shared_ptr<pdat::CellData<double> > > data_enthalpies;
        data_enthalpies.reserve(d_num_species);
        
        for (int si = 0; si < d_num_species; si++)
        {
            data_enthalpies.push_back(boost::make_shared<pdat::CellData<double> >(
                d_interior_box, 1, d_num_subghosts_diffusivities));
        }
        
        std::vector<double*> h;
        h.reserve(d_num_species);
        for (int si = 0; si < d_num_species; si++)
        {
            h.push_back(data_enthalpies[si]->getPointer(0));
        }
        
        if (d_dim == tbox::Dimension(1))
        {
            std::vector<double> species_thermo_properties;
            std::vector<double*> species_thermo_properties_ptr;
            std::vector<const double*> species_thermo_properties_const_ptr;
            
            const int num_thermo_properties = d_equation_of_state_mixing_rules->
                getNumberOfSpeciesThermodynamicProperties();
            
            species_thermo_properties.resize(num_thermo_properties);
            species_thermo_properties_ptr.reserve(num_thermo_properties);
            species_thermo_properties_const_ptr.reserve(num_thermo_properties);
            
            for (int ti = 0; ti < num_thermo_properties; ti++)
            {
                species_thermo_properties_ptr.push_back(&species_thermo_properties[ti]);
                species_thermo_properties_const_ptr.push_back(&species_thermo_properties[ti]);
            }
            
            for (int si = 0; si < d_num_species; si++)
            {
                d_equation_of_state_mixing_rules->getSpeciesThermodynamicProperties(
                    species_thermo_properties_ptr,
                    si);
                
                for (int i = -d_num_subghosts_diffusivities[0];
                     i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx_diffusivities = i + d_num_subghosts_diffusivities[0];
                    const int idx_pressure = i + d_num_subghosts_pressure[0];
                    const int idx_temperature = i + d_num_subghosts_temperature[0];
                    
                    const double rho_i =
                        d_equation_of_state_mixing_rules->getEquationOfState()->
                            getDensity(
                                &p[idx_pressure],
                                &T[idx_temperature],
                                species_thermo_properties_const_ptr);
                    
                    h[si][idx_diffusivities] =
                        d_equation_of_state_mixing_rules->getEquationOfState()->
                            getEnthalpy(
                                &rho_i,
                                &p[idx_pressure],
                                species_thermo_properties_const_ptr);
                }
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            std::vector<double> species_thermo_properties;
            std::vector<double*> species_thermo_properties_ptr;
            std::vector<const double*> species_thermo_properties_const_ptr;
            
            const int num_thermo_properties = d_equation_of_state_mixing_rules->
                getNumberOfSpeciesThermodynamicProperties();
            
            species_thermo_properties.resize(num_thermo_properties);
            species_thermo_properties_ptr.reserve(num_thermo_properties);
            species_thermo_properties_const_ptr.reserve(num_thermo_properties);
            
            for (int ti = 0; ti < num_thermo_properties; ti++)
            {
                species_thermo_properties_ptr.push_back(&species_thermo_properties[ti]);
                species_thermo_properties_const_ptr.push_back(&species_thermo_properties[ti]);
            }
            
            for (int si = 0; si < d_num_species; si++)
            {
                d_equation_of_state_mixing_rules->getSpeciesThermodynamicProperties(
                    species_thermo_properties_ptr,
                    si);
                
                for (int j = -d_num_subghosts_diffusivities[1];
                     j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_diffusivities[0];
                         i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                         i++)
                    {
                        // Compute the linear indices.
                        const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                            (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0];
                        
                        const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                            (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0];
                        
                        const int idx_temperature = (i + d_num_subghosts_temperature[0]) +
                            (j + d_num_subghosts_temperature[1])*d_subghostcell_dims_temperature[0];
                        
                        const double rho_i =
                            d_equation_of_state_mixing_rules->getEquationOfState()->
                                getDensity(
                                    &p[idx_pressure],
                                    &T[idx_temperature],
                                    species_thermo_properties_const_ptr);
                        
                        h[si][idx_diffusivities] =
                            d_equation_of_state_mixing_rules->getEquationOfState()->
                                getEnthalpy(
                                    &rho_i,
                                    &p[idx_pressure],
                                    species_thermo_properties_const_ptr);
                    }
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            std::vector<double> species_thermo_properties;
            std::vector<double*> species_thermo_properties_ptr;
            std::vector<const double*> species_thermo_properties_const_ptr;
            
            const int num_thermo_properties = d_equation_of_state_mixing_rules->
                getNumberOfSpeciesThermodynamicProperties();
            
            species_thermo_properties.resize(num_thermo_properties);
            species_thermo_properties_ptr.reserve(num_thermo_properties);
            species_thermo_properties_const_ptr.reserve(num_thermo_properties);
            
            for (int ti = 0; ti < num_thermo_properties; ti++)
            {
                species_thermo_properties_ptr.push_back(&species_thermo_properties[ti]);
                species_thermo_properties_const_ptr.push_back(&species_thermo_properties[ti]);
            }
            
            for (int si = 0; si < d_num_species; si++)
            {
                d_equation_of_state_mixing_rules->getSpeciesThermodynamicProperties(
                    species_thermo_properties_ptr,
                    si);
                
                for (int k = -d_num_subghosts_diffusivities[2];
                     k < d_interior_dims[2] + d_num_subghosts_diffusivities[2];
                     k++)
                {
                    for (int j = -d_num_subghosts_diffusivities[1];
                         j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                         j++)
                    {
                        for (int i = -d_num_subghosts_diffusivities[0];
                             i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                             i++)
                        {
                            const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                                (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0] +
                                (k + d_num_subghosts_diffusivities[2])*d_subghostcell_dims_diffusivities[0]*
                                    d_subghostcell_dims_diffusivities[1];
                            
                            const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                                (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0] +
                                (k + d_num_subghosts_pressure[2])*d_subghostcell_dims_pressure[0]*
                                    d_subghostcell_dims_pressure[1];
                            
                            const int idx_temperature = (i + d_num_subghosts_temperature[0]) +
                                (j + d_num_subghosts_temperature[1])*d_subghostcell_dims_temperature[0] +
                                (k + d_num_subghosts_temperature[2])*d_subghostcell_dims_temperature[0]*
                                    d_subghostcell_dims_temperature[1];
                            
                            const double rho_i =
                                d_equation_of_state_mixing_rules->getEquationOfState()->
                                    getDensity(
                                        &p[idx_pressure],
                                        &T[idx_temperature],
                                        species_thermo_properties_const_ptr);
                            
                            h[si][idx_diffusivities] =
                                d_equation_of_state_mixing_rules->getEquationOfState()->
                                    getEnthalpy(
                                        &rho_i,
                                        &p[idx_pressure],
                                        species_thermo_properties_const_ptr);
                        }
                    }
                }
            }
        }
        
        if (d_dim == tbox::Dimension(1))
        {
            d_data_diffusivities.reset(new pdat::CellData<double>(
                d_interior_box,
                2*d_num_species*(d_num_species + 1) + 3,
                d_num_subghosts_diffusivities));
            
            // Get the pointer to cell data of velocity and diffusivities.
            double* u = d_data_velocity->getPointer(0);
            
            std::vector<double*> D_ptr;
            D_ptr.reserve(2*d_num_species*(d_num_species + 1) + 3);
            
            for (int i = 0; i < 2*d_num_species*(d_num_species + 1) + 3; i++)
            {
                D_ptr.push_back(d_data_diffusivities->getPointer(i));
            }
            
            /*
             * Compute the diffusivities.
             */
            
            // Mass equations.
            for (int si = 0; si < d_num_species; si++)
            {
                const int component_idx = si*(d_num_species + 1);
                
                for (int i = -d_num_subghosts_diffusivities[0];
                     i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx_diffusivities = i + d_num_subghosts_diffusivities[0];
                    const int idx_density = i + d_num_subghosts_density[0];
                    
                    D_ptr[component_idx][idx_diffusivities] =
                        -rho[idx_density]*D[si][idx_diffusivities];
                }
            }
            
            for (int si = 0; si < d_num_species; si++)
            {
                for (int sj = 0; sj < d_num_species; sj++)
                {
                    const int component_idx = si*(d_num_species + 1) + sj + 1;
                    
                    for (int i = -d_num_subghosts_diffusivities[0];
                         i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                         i++)
                    {
                        // Compute the linear indices.
                        const int idx_diffusivities = i + d_num_subghosts_diffusivities[0];
                        const int idx_density = i + d_num_subghosts_density[0];
                        const int idx_mass_fraction = i + d_num_subghosts_mass_fraction[0];
                        
                        D_ptr[component_idx][idx_diffusivities] =
                            rho[idx_density]*Y[si][idx_mass_fraction]*D[sj][idx_diffusivities];
                    }
                }
            }
            
            // Momentum and energy equations.
            for (int i = -d_num_subghosts_diffusivities[0];
                 i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                 i++)
            {
                // Compute the linear indices.
                const int idx_diffusivities = i + d_num_subghosts_diffusivities[0];
                const int idx_velocity = i + d_num_subghosts_velocity[0];
                
                D_ptr[d_num_species*(d_num_species + 1)][idx_diffusivities] =
                    -(4.0/3.0*mu[idx_diffusivities] + mu_v[idx_diffusivities]);
                D_ptr[d_num_species*(d_num_species + 1) + 1][idx_diffusivities] =
                    -u[idx_velocity]*(4.0/3.0*mu[idx_diffusivities] + mu_v[idx_diffusivities]);
                D_ptr[d_num_species*(d_num_species + 1) + 2][idx_diffusivities] =
                    -kappa[idx_diffusivities];
            }
            
            // Energy equation.
            for (int si = 0; si < d_num_species; si++)
            {
                const int component_idx =
                    d_num_species*(d_num_species + 1) + 3 + si*(d_num_species + 1);
                
                for (int i = -d_num_subghosts_diffusivities[0];
                     i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx_diffusivities = i + d_num_subghosts_diffusivities[0];
                    const int idx_density = i + d_num_subghosts_density[0];
                    
                    D_ptr[component_idx][idx_diffusivities] =
                        -rho[idx_density]*D[si][idx_diffusivities]*
                            h[si][idx_diffusivities];
                }
            }
            
            for (int si = 0; si < d_num_species; si++)
            {
                for (int sj = 0; sj < d_num_species; sj++)
                {
                    const int component_idx =
                        d_num_species*(d_num_species + 1) + 4 + si*(d_num_species + 1) + sj;
                    
                    for (int i = -d_num_subghosts_diffusivities[0];
                         i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                         i++)
                    {
                        // Compute the linear indices.
                        const int idx_diffusivities = i + d_num_subghosts_diffusivities[0];
                        const int idx_density = i + d_num_subghosts_density[0];
                        const int idx_mass_fraction = i + d_num_subghosts_mass_fraction[0];
                        
                        D_ptr[component_idx][idx_diffusivities] =
                            rho[idx_density]*Y[si][idx_mass_fraction]*D[sj][idx_diffusivities]*
                                h[si][idx_diffusivities];
                    }
                }
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            d_data_diffusivities.reset(new pdat::CellData<double>(
                d_interior_box,
                2*d_num_species*(d_num_species + 1) + 10,
                d_num_subghosts_diffusivities));
            
            // Get the pointer to cell data of velocity and diffusivities.
            double* u = d_data_velocity->getPointer(0);
            double* v = d_data_velocity->getPointer(1);
            
            std::vector<double*> D_ptr;
            D_ptr.reserve(2*d_num_species*(d_num_species + 1) + 10);
            
            for (int i = 0; i < 2*d_num_species*(d_num_species + 1) + 10; i++)
            {
                D_ptr.push_back(d_data_diffusivities->getPointer(i));
            }
            
            /*
             * Compute the diffusivities.
             */
            
            // Mass equations.
            for (int si = 0; si < d_num_species; si++)
            {
                const int component_idx = si*(d_num_species + 1);
                
                for (int j = -d_num_subghosts_diffusivities[1];
                     j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_diffusivities[0];
                         i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                         i++)
                    {
                        // Compute the linear indices.
                        const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                            (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0];
                        
                        const int idx_density = (i + d_num_subghosts_density[0]) +
                            (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0];
                        
                        D_ptr[component_idx][idx_diffusivities] =
                            -rho[idx_density]*D[si][idx_diffusivities];
                    }
                }
            }
            
            for (int si = 0; si < d_num_species; si++)
            {
                for (int sj = 0; sj < d_num_species; sj++)
                {
                    const int component_idx = si*(d_num_species + 1) + sj + 1;
                    
                    for (int j = -d_num_subghosts_diffusivities[1];
                         j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                         j++)
                    {
                        for (int i = -d_num_subghosts_diffusivities[0];
                             i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                             i++)
                        {
                            // Compute the linear indices.
                            const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                                (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0];
                            
                            const int idx_density = (i + d_num_subghosts_density[0]) +
                                (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0];
                            
                            const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                                (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0];
                            
                            D_ptr[component_idx][idx_diffusivities] =
                                rho[idx_density]*Y[si][idx_mass_fraction]*D[sj][idx_diffusivities];
                        }
                    }
                }
            }
            
            // Momentum and energy equations.
            for (int j = -d_num_subghosts_diffusivities[1];
                 j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                 j++)
            {
                for (int i = -d_num_subghosts_diffusivities[0];
                     i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                        (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0];
                    
                    const int idx_velocity = (i + d_num_subghosts_velocity[0]) +
                        (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                    
                    D_ptr[d_num_species*(d_num_species + 1)][idx_diffusivities] =
                        -(4.0/3.0*mu[idx_diffusivities] + mu_v[idx_diffusivities]);
                    D_ptr[d_num_species*(d_num_species + 1) + 1][idx_diffusivities] =
                        2.0/3.0*mu[idx_diffusivities] - mu_v[idx_diffusivities];
                    D_ptr[d_num_species*(d_num_species + 1) + 2][idx_diffusivities] =
                        -mu[idx_diffusivities];
                    D_ptr[d_num_species*(d_num_species + 1) + 3][idx_diffusivities] =
                        -u[idx_velocity]*(4.0/3.0*mu[idx_diffusivities] + mu_v[idx_diffusivities]);
                    D_ptr[d_num_species*(d_num_species + 1) + 4][idx_diffusivities] =
                        -v[idx_velocity]*(4.0/3.0*mu[idx_diffusivities] + mu_v[idx_diffusivities]);
                    D_ptr[d_num_species*(d_num_species + 1) + 5][idx_diffusivities] =
                        u[idx_velocity]*(2.0/3.0*mu[idx_diffusivities] - mu_v[idx_diffusivities]);
                    D_ptr[d_num_species*(d_num_species + 1) + 6][idx_diffusivities] =
                        v[idx_velocity]*(2.0/3.0*mu[idx_diffusivities] - mu_v[idx_diffusivities]);
                    D_ptr[d_num_species*(d_num_species + 1) + 7][idx_diffusivities] =
                        -u[idx_velocity]*mu[idx_diffusivities];
                    D_ptr[d_num_species*(d_num_species + 1) + 8][idx_diffusivities] =
                        -v[idx_velocity]*mu[idx_diffusivities];
                    D_ptr[d_num_species*(d_num_species + 1) + 9][idx_diffusivities] =
                        -kappa[idx_diffusivities];
                }
            }
            
            // Energy equation.
            for (int si = 0; si < d_num_species; si++)
            {
                const int component_idx =
                    d_num_species*(d_num_species + 1) + 10 + si*(d_num_species + 1);
                
                for (int j = -d_num_subghosts_diffusivities[1];
                     j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_diffusivities[0];
                         i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                         i++)
                    {
                        // Compute the linear indices.
                        const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                            (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0];
                        
                        const int idx_density = (i + d_num_subghosts_density[0]) +
                            (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0];
                        
                        D_ptr[component_idx][idx_diffusivities] =
                            -rho[idx_density]*D[si][idx_diffusivities]*
                                h[si][idx_diffusivities];
                    }
                }
            }
            
            for (int si = 0; si < d_num_species; si++)
            {
                for (int sj = 0; sj < d_num_species; sj++)
                {
                    const int component_idx =
                        d_num_species*(d_num_species + 1) + 11 + si*(d_num_species + 1) + sj;
                    
                    for (int j = -d_num_subghosts_diffusivities[1];
                         j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                         j++)
                    {
                        for (int i = -d_num_subghosts_diffusivities[0];
                             i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                             i++)
                        {
                            // Compute the linear indices.
                            const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                                (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0];
                            
                            const int idx_density = (i + d_num_subghosts_density[0]) +
                                (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0];
                            
                            const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                                (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0];
                            
                            D_ptr[component_idx][idx_diffusivities] =
                                rho[idx_density]*Y[si][idx_mass_fraction]*D[sj][idx_diffusivities]*
                                    h[si][idx_diffusivities];
                        }
                    }
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            d_data_diffusivities.reset(new pdat::CellData<double>(
                d_interior_box,
                2*d_num_species*(d_num_species + 1) + 13,
                d_num_subghosts_diffusivities));
            
            // Get the pointer to cell data of velocity and diffusivities.
            double* u = d_data_velocity->getPointer(0);
            double* v = d_data_velocity->getPointer(1);
            double* w = d_data_velocity->getPointer(2);
            
            std::vector<double*> D_ptr;
            D_ptr.reserve(2*d_num_species*(d_num_species + 1) + 13);
            
            for (int i = 0; i < 2*d_num_species*(d_num_species + 1) + 13; i++)
            {
                D_ptr.push_back(d_data_diffusivities->getPointer(i));
            }
            
            /*
             * Compute the diffusivities.
             */
            
            //Mass equations.
            for (int si = 0; si < d_num_species; si++)
            {
                const int component_idx = si*(d_num_species + 1);
                
                for (int k = -d_num_subghosts_diffusivities[2];
                     k < d_interior_dims[2] + d_num_subghosts_diffusivities[2];
                     k++)
                {
                    for (int j = -d_num_subghosts_diffusivities[1];
                         j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                         j++)
                    {
                        for (int i = -d_num_subghosts_diffusivities[0];
                             i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                             i++)
                        {
                            // Compute the linear indices.
                            const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                                (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0] +
                                (k + d_num_subghosts_diffusivities[2])*d_subghostcell_dims_diffusivities[0]*
                                    d_subghostcell_dims_diffusivities[1];
                            
                            const int idx_density = (i + d_num_subghosts_density[0]) +
                                (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0] +
                                (k + d_num_subghosts_density[2])*d_subghostcell_dims_density[0]*
                                    d_subghostcell_dims_density[1];
                            
                            D_ptr[component_idx][idx_diffusivities] =
                                -rho[idx_density]*D[si][idx_diffusivities];
                        }
                    }
                }
            }
            
            for (int si = 0; si < d_num_species; si++)
            {
                for (int sj = 0; sj < d_num_species; sj++)
                {
                    const int component_idx = si*(d_num_species + 1) + sj + 1;
                    
                    for (int k = -d_num_subghosts_diffusivities[2];
                         k < d_interior_dims[2] + d_num_subghosts_diffusivities[2];
                         k++)
                    {
                        for (int j = -d_num_subghosts_diffusivities[1];
                             j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                             j++)
                        {
                            for (int i = -d_num_subghosts_diffusivities[0];
                                 i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                                 i++)
                            {
                                // Compute the linear indices.
                                const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                                    (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0] +
                                    (k + d_num_subghosts_diffusivities[2])*d_subghostcell_dims_diffusivities[0]*
                                        d_subghostcell_dims_diffusivities[1];
                                
                                const int idx_density = (i + d_num_subghosts_density[0]) +
                                    (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0] +
                                    (k + d_num_subghosts_density[2])*d_subghostcell_dims_density[0]*
                                        d_subghostcell_dims_density[1];
                                
                                const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                                    (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0] +
                                    (k + d_num_subghosts_mass_fraction[2])*d_subghostcell_dims_mass_fraction[0]*
                                        d_subghostcell_dims_mass_fraction[1];
                                
                                D_ptr[component_idx][idx_diffusivities] =
                                    rho[idx_density]*Y[si][idx_mass_fraction]*D[sj][idx_diffusivities];
                            }
                        }
                    }
                }
            }
            
            // Momentum and energy equations.
            for (int k = -d_num_subghosts_diffusivities[2];
                 k < d_interior_dims[2] + d_num_subghosts_diffusivities[2];
                 k++)
            {
                for (int j = -d_num_subghosts_diffusivities[1];
                     j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_diffusivities[0];
                         i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                         i++)
                    {
                        // Compute the linear indices.
                        const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                            (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0] +
                            (k + d_num_subghosts_diffusivities[2])*d_subghostcell_dims_diffusivities[0]*
                                d_subghostcell_dims_diffusivities[1];
                        
                        const int idx_velocity = (i + d_num_subghosts_velocity[0]) +
                            (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                            (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                d_subghostcell_dims_velocity[1];
                        
                        D_ptr[d_num_species*(d_num_species + 1)][idx_diffusivities] =
                            -(4.0/3.0*mu[idx_diffusivities] + mu_v[idx_diffusivities]);
                        D_ptr[d_num_species*(d_num_species + 1) + 1][idx_diffusivities] =
                            2.0/3.0*mu[idx_diffusivities] - mu_v[idx_diffusivities];
                        D_ptr[d_num_species*(d_num_species + 1) + 2][idx_diffusivities] =
                            -mu[idx_diffusivities];
                        D_ptr[d_num_species*(d_num_species + 1) + 3][idx_diffusivities] =
                            -u[idx_velocity]*(4.0/3.0*mu[idx_diffusivities] + mu_v[idx_diffusivities]);
                        D_ptr[d_num_species*(d_num_species + 1) + 4][idx_diffusivities] =
                            -v[idx_velocity]*(4.0/3.0*mu[idx_diffusivities] + mu_v[idx_diffusivities]);
                        D_ptr[d_num_species*(d_num_species + 1) + 5][idx_diffusivities] =
                            -w[idx_velocity]*(4.0/3.0*mu[idx_diffusivities] + mu_v[idx_diffusivities]);
                        D_ptr[d_num_species*(d_num_species + 1) + 6][idx_diffusivities] =
                            u[idx_velocity]*(2.0/3.0*mu[idx_diffusivities] - mu_v[idx_diffusivities]);
                        D_ptr[d_num_species*(d_num_species + 1) + 7][idx_diffusivities] =
                            v[idx_velocity]*(2.0/3.0*mu[idx_diffusivities] - mu_v[idx_diffusivities]);
                        D_ptr[d_num_species*(d_num_species + 1) + 8][idx_diffusivities] =
                            w[idx_velocity]*(2.0/3.0*mu[idx_diffusivities] - mu_v[idx_diffusivities]);
                        D_ptr[d_num_species*(d_num_species + 1) + 9][idx_diffusivities] =
                            -u[idx_velocity]*mu[idx_diffusivities];
                        D_ptr[d_num_species*(d_num_species + 1) + 10][idx_diffusivities] =
                            -v[idx_velocity]*mu[idx_diffusivities];
                        D_ptr[d_num_species*(d_num_species + 1) + 11][idx_diffusivities] =
                            -w[idx_velocity]*mu[idx_diffusivities];
                        D_ptr[d_num_species*(d_num_species + 1) + 12][idx_diffusivities] =
                            -kappa[idx_diffusivities];
                    }
                }
            }
            
            // Energy equation.
            for (int si = 0; si < d_num_species; si++)
            {
                const int component_idx =
                    d_num_species*(d_num_species + 1) + 13 + si*(d_num_species + 1);
                
                for (int k = -d_num_subghosts_diffusivities[2];
                     k < d_interior_dims[2] + d_num_subghosts_diffusivities[2];
                     k++)
                {
                    for (int j = -d_num_subghosts_diffusivities[1];
                         j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                         j++)
                    {
                        for (int i = -d_num_subghosts_diffusivities[0];
                             i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                             i++)
                        {
                            // Compute the linear indices.
                            const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                                (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0] +
                                (k + d_num_subghosts_diffusivities[2])*d_subghostcell_dims_diffusivities[0]*
                                    d_subghostcell_dims_diffusivities[1];
                            
                            const int idx_density = (i + d_num_subghosts_density[0]) +
                                (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0] +
                                (k + d_num_subghosts_density[2])*d_subghostcell_dims_density[0]*
                                    d_subghostcell_dims_density[1];
                            
                            D_ptr[component_idx][idx_diffusivities] =
                                -rho[idx_density]*D[si][idx_diffusivities]*
                                    h[si][idx_diffusivities];
                        }
                    }
                }
            }
            
            for (int si = 0; si < d_num_species; si++)
            {
                for (int sj = 0; sj < d_num_species; sj++)
                {
                    const int component_idx =
                        d_num_species*(d_num_species + 1) + 14 + si*(d_num_species + 1) + sj;
                    
                    for (int k = -d_num_subghosts_diffusivities[2];
                         k < d_interior_dims[2] + d_num_subghosts_diffusivities[2];
                         k++)
                    {
                        for (int j = -d_num_subghosts_diffusivities[1];
                             j < d_interior_dims[1] + d_num_subghosts_diffusivities[1];
                             j++)
                        {
                            for (int i = -d_num_subghosts_diffusivities[0];
                                 i < d_interior_dims[0] + d_num_subghosts_diffusivities[0];
                                 i++)
                            {
                                // Compute the linear indices.
                                const int idx_diffusivities = (i + d_num_subghosts_diffusivities[0]) +
                                    (j + d_num_subghosts_diffusivities[1])*d_subghostcell_dims_diffusivities[0] +
                                    (k + d_num_subghosts_diffusivities[2])*d_subghostcell_dims_diffusivities[0]*
                                        d_subghostcell_dims_diffusivities[1];
                                
                                const int idx_density = (i + d_num_subghosts_density[0]) +
                                    (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0] +
                                    (k + d_num_subghosts_density[2])*d_subghostcell_dims_density[0]*
                                        d_subghostcell_dims_density[1];
                                
                                const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                                    (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0] +
                                    (k + d_num_subghosts_mass_fraction[2])*d_subghostcell_dims_mass_fraction[0]*
                                        d_subghostcell_dims_mass_fraction[1];
                                
                                D_ptr[component_idx][idx_diffusivities] =
                                    rho[idx_density]*Y[si][idx_mass_fraction]*D[sj][idx_diffusivities]*
                                        h[si][idx_diffusivities];
                            }
                        }
                    }
                }
            }
        }
        
        data_mass_diffusivities.reset();
        data_shear_viscosity.reset();
        data_bulk_viscosity.reset();
        data_thermal_conductivity.reset();
        data_enthalpies.clear();
    }
    
    if (d_dim == tbox::Dimension(1))
    {
        switch (flux_direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(d_num_species + 1);
                            diffusivities_component_idx[si].resize(d_num_species + 1);
                            
                            diffusivities_data[si][0] = d_data_diffusivities;
                            diffusivities_component_idx[si][0] = si*(d_num_species + 1);
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                diffusivities_data[si][1 + sj] = d_data_diffusivities;
                                diffusivities_component_idx[si][1 + sj] = si*(d_num_species + 1) + sj + 1;
                            }
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -(4/3*mu + mu_v).
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] =
                            d_num_species*(d_num_species + 1);
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 1].resize(
                            2 + d_num_species*(d_num_species + 1));
                        diffusivities_component_idx[d_num_species + 1].resize(
                            2 + d_num_species*(d_num_species + 1));
                        
                        // -u*(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] =
                            d_num_species*(d_num_species + 1) + 1;
                        
                        // -kappa.
                        diffusivities_data[d_num_species + 1][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][1] =
                            d_num_species*(d_num_species + 1) + 2;
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[d_num_species + 1][2 + si*(d_num_species + 1)] =
                                d_data_diffusivities;
                            diffusivities_component_idx[d_num_species + 1][2 + si*(d_num_species + 1)] =
                                d_num_species*(d_num_species + 1) + 3 + si*(d_num_species + 1);
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                diffusivities_data[d_num_species + 1][3 + si*(d_num_species + 1) + sj] =
                                    d_data_diffusivities;
                                diffusivities_component_idx[d_num_species + 1][3 + si*(d_num_species + 1) + sj] =
                                    d_num_species*(d_num_species + 1) + 4 + si*(d_num_species + 1) + sj;
                            }
                        }
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelFourEqnConservative::getDiffusiveFluxDiffusivities()\n"
                            << "There are only x-direction for one-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::getDiffusiveFluxDiffusivities()\n"
                    << "There are only x-direction for one-dimensional problem."
                    << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(2))
    {
        switch (flux_direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(d_num_species + 1);
                            diffusivities_component_idx[si].resize(d_num_species + 1);
                            
                            diffusivities_data[si][0] = d_data_diffusivities;
                            diffusivities_component_idx[si][0] = si*(d_num_species + 1);
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                diffusivities_data[si][1 + sj] = d_data_diffusivities;
                                diffusivities_component_idx[si][1 + sj] = si*(d_num_species + 1) + sj + 1;
                            }
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -(4/3*mu + mu_v).
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] =
                            d_num_species*(d_num_species + 1);
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] =
                            d_num_species*(d_num_species + 1) + 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 2].resize(
                            3 + d_num_species*(d_num_species + 1));
                        diffusivities_component_idx[d_num_species + 2].resize(
                            3 + d_num_species*(d_num_species + 1));
                        
                        // -u*(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] =
                            d_num_species*(d_num_species + 1) + 3;
                        
                        // -v*mu.
                        diffusivities_data[d_num_species + 2][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][1] =
                            d_num_species*(d_num_species + 1) + 8;
                        
                        // -kappa.
                        diffusivities_data[d_num_species + 2][2] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][2] =
                            d_num_species*(d_num_species + 1) + 9;
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[d_num_species + 2][3 + si*(d_num_species + 1)] =
                                d_data_diffusivities;
                            diffusivities_component_idx[d_num_species + 2][3 + si*(d_num_species + 1)] =
                                d_num_species*(d_num_species + 1) + 10 + si*(d_num_species + 1);
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                diffusivities_data[d_num_species + 2][4 + si*(d_num_species + 1) + sj] =
                                    d_data_diffusivities;
                                diffusivities_component_idx[d_num_species + 2][4 + si*(d_num_species + 1) + sj] =
                                    d_num_species*(d_num_species + 1) + 11 + si*(d_num_species + 1) + sj;
                            }
                        }
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // 2/3*mu - mu_v.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] =
                            d_num_species*(d_num_species + 1) + 1;
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] =
                            d_num_species*(d_num_species + 1) + 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 2].resize(2);
                        diffusivities_component_idx[d_num_species + 2].resize(2);
                        
                        // -v*mu.
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] =
                            d_num_species*(d_num_species + 1) + 8;
                        
                        // u*(2/3*mu - mu_v).
                        diffusivities_data[d_num_species + 2][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][1] =
                            d_num_species*(d_num_species + 1) + 5;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelFourEqnConservative::getDiffusiveFluxDiffusivities()\n"
                            << "There are only x-direction and y-direction for two-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            case DIRECTION::Y_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] =
                            d_num_species*(d_num_species + 1) + 2;
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // 2/3*mu - mu_v.
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] =
                            d_num_species*(d_num_species + 1) + 1;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 2].resize(2);
                        diffusivities_component_idx[d_num_species + 2].resize(2);
                        
                        // v*(2/3*mu - mu_v).
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] =
                            d_num_species*(d_num_species + 1) + 6;
                        
                        // -u*mu.
                        diffusivities_data[d_num_species + 2][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][1] =
                            d_num_species*(d_num_species + 1) + 7;
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(d_num_species + 1);
                            diffusivities_component_idx[si].resize(d_num_species + 1);
                            
                            diffusivities_data[si][0] = d_data_diffusivities;
                            diffusivities_component_idx[si][0] = si*(d_num_species + 1);
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                diffusivities_data[si][1 + sj] = d_data_diffusivities;
                                diffusivities_component_idx[si][1 + sj] = si*(d_num_species + 1) + sj + 1;
                            }
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] =
                            d_num_species*(d_num_species + 1) + 2;
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // -(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] =
                            d_num_species*(d_num_species + 1);
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 2].resize(
                            3 + d_num_species*(d_num_species + 1));
                        diffusivities_component_idx[d_num_species + 2].resize(
                            3 + d_num_species*(d_num_species + 1));
                        
                        // -u*mu.
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] =
                            d_num_species*(d_num_species + 1) + 7;
                        
                        // -v*(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 2][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][1] =
                            d_num_species*(d_num_species + 1) + 4;
                        
                        // -kappa.
                        diffusivities_data[d_num_species + 2][2] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][2] =
                            d_num_species*(d_num_species + 1) + 9;
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[d_num_species + 2][3 + si*(d_num_species + 1)] =
                                d_data_diffusivities;
                            diffusivities_component_idx[d_num_species + 2][3 + si*(d_num_species + 1)] =
                                d_num_species*(d_num_species + 1) + 10 + si*(d_num_species + 1);
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                diffusivities_data[d_num_species + 2][4 + si*(d_num_species + 1) + sj] =
                                    d_data_diffusivities;
                                diffusivities_component_idx[d_num_species + 2][4 + si*(d_num_species + 1) + sj] =
                                    d_num_species*(d_num_species + 1) + 11 + si*(d_num_species + 1) + sj;
                            }
                        }
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelFourEqnConservative::getDiffusiveFluxDiffusivities()\n"
                            << "There are only x-direction and y-direction for two-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::getDiffusiveFluxDiffusivities()\n"
                    << "There are only x-direction and y-direction for two-dimensional problem."
                    << std::endl);
            }
        }
    }
    else if (d_dim == tbox::Dimension(3))
    {
        switch (flux_direction)
        {
            case DIRECTION::X_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(d_num_species + 1);
                            diffusivities_component_idx[si].resize(d_num_species + 1);
                            
                            diffusivities_data[si][0] = d_data_diffusivities;
                            diffusivities_component_idx[si][0] = si*(d_num_species + 1);
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                diffusivities_data[si][1 + sj] = d_data_diffusivities;
                                diffusivities_component_idx[si][1 + sj] = si*(d_num_species + 1) + sj + 1;
                            }
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -(4/3*mu + mu_v).
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] =
                            d_num_species*(d_num_species + 1);
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] =
                            d_num_species*(d_num_species + 1) + 2;
                        
                        diffusivities_data[d_num_species + 2].resize(1);
                        diffusivities_component_idx[d_num_species + 2].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] =
                            d_num_species*(d_num_species + 1) + 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(
                            4 + d_num_species*(d_num_species + 1));
                        diffusivities_component_idx[d_num_species + 3].resize(
                            4 + d_num_species*(d_num_species + 1));
                        
                        // -u*(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] =
                            d_num_species*(d_num_species + 1) + 3;
                        
                        // -v*mu.
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] =
                            d_num_species*(d_num_species + 1) + 10;
                        
                        // -w*mu.
                        diffusivities_data[d_num_species + 3][2] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][2] =
                            d_num_species*(d_num_species + 1) + 11;
                        
                        // -kappa.
                        diffusivities_data[d_num_species + 3][3] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][3] =
                            d_num_species*(d_num_species + 1) + 12;
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[d_num_species + 3][4 + si*(d_num_species + 1)] =
                                d_data_diffusivities;
                            diffusivities_component_idx[d_num_species + 3][4 + si*(d_num_species + 1)] =
                                d_num_species*(d_num_species + 1) + 13 + si*(d_num_species + 1);
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                diffusivities_data[d_num_species + 3][5 + si*(d_num_species + 1) + sj] =
                                    d_data_diffusivities;
                                diffusivities_component_idx[d_num_species + 3][5 + si*(d_num_species + 1) + sj] =
                                    d_num_species*(d_num_species + 1) + 14 + si*(d_num_species + 1) + sj;
                            }
                        }
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // 2/3*mu - mu_v.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] =
                            d_num_species*(d_num_species + 1) + 1;
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] =
                            d_num_species*(d_num_species + 1) + 2;
                        
                        diffusivities_data[d_num_species + 2].resize(0);
                        diffusivities_component_idx[d_num_species + 2].resize(0);
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(2);
                        diffusivities_component_idx[d_num_species + 3].resize(2);
                        
                        // -v*mu.
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] =
                            d_num_species*(d_num_species + 1) + 10;
                        
                        // u*(2/3*mu - mu_v).
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] =
                            d_num_species*(d_num_species + 1) + 6;
                        
                        break;
                    }
                    case DIRECTION::Z_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // 2/3*mu - mu_v.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] =
                            d_num_species*(d_num_species + 1) + 1;
                        
                        diffusivities_data[d_num_species + 1].resize(0);
                        diffusivities_component_idx[d_num_species + 1].resize(0);
                        
                        diffusivities_data[d_num_species + 2].resize(1);
                        diffusivities_component_idx[d_num_species + 2].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] =
                            d_num_species*(d_num_species + 1) + 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(2);
                        diffusivities_component_idx[d_num_species + 3].resize(2);
                        
                        // -w*mu.
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] =
                            d_num_species*(d_num_species + 1) + 11;
                        
                        // u*(2/3*mu - mu_v).
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] =
                            d_num_species*(d_num_species + 1) + 6;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelFourEqnConservative::getDiffusiveFluxDiffusivities()\n"
                            << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            case DIRECTION::Y_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] =
                            d_num_species*(d_num_species + 1) + 2;
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // 2/3*mu - mu_v.
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] =
                            d_num_species*(d_num_species + 1) + 1;
                        
                        diffusivities_data[d_num_species + 2].resize(0);
                        diffusivities_component_idx[d_num_species + 2].resize(0);
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(2);
                        diffusivities_component_idx[d_num_species + 3].resize(2);
                        
                        // v*(2/3*mu - mu_v).
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] =
                            d_num_species*(d_num_species + 1) + 7;
                        
                        // -u*mu.
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] =
                            d_num_species*(d_num_species + 1) + 9;
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(d_num_species + 1);
                            diffusivities_component_idx[si].resize(d_num_species + 1);
                            
                            diffusivities_data[si][0] = d_data_diffusivities;
                            diffusivities_component_idx[si][0] = si*(d_num_species + 1);
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                diffusivities_data[si][1 + sj] = d_data_diffusivities;
                                diffusivities_component_idx[si][1 + sj] = si*(d_num_species + 1) + sj + 1;
                            }
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] =
                            d_num_species*(d_num_species + 1) + 2;
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // -(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] =
                            d_num_species*(d_num_species + 1);
                        
                        diffusivities_data[d_num_species + 2].resize(1);
                        diffusivities_component_idx[d_num_species + 2].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] =
                            d_num_species*(d_num_species + 1) + 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(
                            4 + d_num_species*(d_num_species + 1));
                        diffusivities_component_idx[d_num_species + 3].resize(
                            4 + d_num_species*(d_num_species + 1));
                        
                        // -u*mu.
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] =
                            d_num_species*(d_num_species + 1) + 9;
                        
                        // -v*(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] =
                            d_num_species*(d_num_species + 1) + 4;
                        
                        // -w*mu.
                        diffusivities_data[d_num_species + 3][2] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][2] =
                            d_num_species*(d_num_species + 1) + 11;
                        
                        // -kappa.
                        diffusivities_data[d_num_species + 3][3] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][3] =
                            d_num_species*(d_num_species + 1) + 12;
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[d_num_species + 3][4 + si*(d_num_species + 1)] =
                                d_data_diffusivities;
                            diffusivities_component_idx[d_num_species + 3][4 + si*(d_num_species + 1)] =
                                d_num_species*(d_num_species + 1) + 13 + si*(d_num_species + 1);
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                diffusivities_data[d_num_species + 3][5 + si*(d_num_species + 1) + sj] =
                                    d_data_diffusivities;
                                diffusivities_component_idx[d_num_species + 3][5 + si*(d_num_species + 1) + sj] =
                                    d_num_species*(d_num_species + 1) + 14 + si*(d_num_species + 1) + sj;
                            }
                        }
                        
                        break;
                    }
                    case DIRECTION::Z_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(0);
                        diffusivities_component_idx[d_num_species].resize(0);
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // 2/3*(mu - mu_v).
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] =
                            d_num_species*(d_num_species + 1) + 1;
                        
                        diffusivities_data[d_num_species + 2].resize(1);
                        diffusivities_component_idx[d_num_species + 2].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] =
                            d_num_species*(d_num_species + 1) + 2;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(2);
                        diffusivities_component_idx[d_num_species + 3].resize(2);
                        
                        // -w*u.
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] =
                            d_num_species*(d_num_species + 1) + 11;
                        
                        // v*(2/3*mu - mu_v).
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] =
                            d_num_species*(d_num_species + 1) + 7;
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelFourEqnConservative::getDiffusiveFluxDiffusivities()\n"
                            << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            case DIRECTION::Z_DIRECTION:
            {
                switch (derivative_direction)
                {
                    case DIRECTION::X_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] =
                            d_num_species*(d_num_species + 1) + 2;
                        
                        diffusivities_data[d_num_species + 1].resize(0);
                        diffusivities_component_idx[d_num_species + 1].resize(0);
                        
                        diffusivities_data[d_num_species + 2].resize(1);
                        diffusivities_component_idx[d_num_species + 2].resize(1);
                        
                        // 2/3*mu - mu_v.
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] =
                            d_num_species*(d_num_species + 1) + 1;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(2);
                        diffusivities_component_idx[d_num_species + 3].resize(2);
                        
                        // w*(2/3*mu - mu_v).
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] =
                            d_num_species*(d_num_species + 1) + 8;
                        
                        // -u*mu.
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] =
                            d_num_species*(d_num_species + 1) + 9;
                        
                        break;
                    }
                    case DIRECTION::Y_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(0);
                            diffusivities_component_idx[si].resize(0);
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(0);
                        diffusivities_component_idx[d_num_species].resize(0);
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] =
                            d_num_species*(d_num_species + 1) + 2;
                        
                        diffusivities_data[d_num_species + 2].resize(1);
                        diffusivities_component_idx[d_num_species + 2].resize(1);
                        
                        // 2/3*mu - mu_v.
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] =
                            d_num_species*(d_num_species + 1) + 1;
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(2);
                        diffusivities_component_idx[d_num_species + 3].resize(2);
                        
                        // w*(2/3*mu - mu_v).
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] =
                            d_num_species*(d_num_species + 1) + 8;
                        
                        // -v*mu.
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] =
                            d_num_species*(d_num_species + 1) + 10;
                        
                        break;
                    }
                    case DIRECTION::Z_DIRECTION:
                    {
                        /*
                         * Mass equations.
                         */
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[si].resize(d_num_species + 1);
                            diffusivities_component_idx[si].resize(d_num_species + 1);
                            
                            diffusivities_data[si][0] = d_data_diffusivities;
                            diffusivities_component_idx[si][0] = si*(d_num_species + 1);
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                diffusivities_data[si][1 + sj] = d_data_diffusivities;
                                diffusivities_component_idx[si][1 + sj] = si*(d_num_species + 1) + sj + 1;
                            }
                        }
                        
                        /*
                         * Momentum equation.
                         */
                        
                        diffusivities_data[d_num_species].resize(1);
                        diffusivities_component_idx[d_num_species].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species][0] =
                            d_num_species*(d_num_species + 1) + 2;
                        
                        diffusivities_data[d_num_species + 1].resize(1);
                        diffusivities_component_idx[d_num_species + 1].resize(1);
                        
                        // -mu.
                        diffusivities_data[d_num_species + 1][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 1][0] =
                            d_num_species*(d_num_species + 1) + 2;
                        
                        diffusivities_data[d_num_species + 2].resize(1);
                        diffusivities_component_idx[d_num_species + 2].resize(1);
                        
                        // -(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 2][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 2][0] =
                            d_num_species*(d_num_species + 1);
                        
                        /*
                         * Energy equation.
                         */
                        
                        diffusivities_data[d_num_species + 3].resize(
                            4 + d_num_species*(d_num_species + 1));
                        diffusivities_component_idx[d_num_species + 3].resize(
                            4 + d_num_species*(d_num_species + 1));
                        
                        // -u*mu.
                        diffusivities_data[d_num_species + 3][0] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][0] =
                            d_num_species*(d_num_species + 1) + 9;
                        
                        // -v*mu.
                        diffusivities_data[d_num_species + 3][1] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][1] =
                            d_num_species*(d_num_species + 1) + 10;
                        
                        // -w*(4/3*mu + mu_v).
                        diffusivities_data[d_num_species + 3][2] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][2] =
                            d_num_species*(d_num_species + 1) + 5;
                        
                        // -kappa.
                        diffusivities_data[d_num_species + 3][3] = d_data_diffusivities;
                        diffusivities_component_idx[d_num_species + 3][3] =
                            d_num_species*(d_num_species + 1) + 12;
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            diffusivities_data[d_num_species + 3][4 + si*(d_num_species + 1)] =
                                d_data_diffusivities;
                            diffusivities_component_idx[d_num_species + 3][4 + si*(d_num_species + 1)] =
                                d_num_species*(d_num_species + 1) + 13 + si*(d_num_species + 1);
                            
                            for (int sj = 0; sj < d_num_species; sj++)
                            {
                                diffusivities_data[d_num_species + 3][5 + si*(d_num_species + 1) + sj] =
                                    d_data_diffusivities;
                                diffusivities_component_idx[d_num_species + 3][5 + si*(d_num_species + 1) + sj] =
                                    d_num_species*(d_num_species + 1) + 14 + si*(d_num_species + 1) + sj;
                            }
                        }
                        
                        break;
                    }
                    default:
                    {
                        TBOX_ERROR(d_object_name
                            << ": FlowModelFourEqnConservative::getDiffusiveFluxDiffusivities()\n"
                            << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                            << std::endl);
                    }
                }
                
                break;
            }
            default:
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::getDiffusiveFluxDiffusivities()\n"
                    << "There are only x-direction, y-direction and z-direction for three-dimensional problem."
                    << std::endl);
            }
        }
    }
}


/*
 * Compute derived plot quantities registered with the VisIt data writers from data that
 * is maintained on each patch in the hierarchy.
 */
bool
FlowModelFourEqnConservative::packDerivedDataIntoDoubleBuffer(
    double* buffer,
    const hier::Patch& patch,
    const hier::Box& region,
    const std::string& variable_name,
    int depth_id,
    double simulation_time) const
{
    if (!d_plot_context)
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::"
            << "packDerivedDataIntoDoubleBuffer()\n"
            << "The plotting context is not set yet."
            << std::endl);
    }
    
    NULL_USE(simulation_time);
    
#ifdef DEBUG_CHECK_ASSERTIONS
    TBOX_ASSERT((region * patch.getBox()).isSpatiallyEqual(region));
#endif
    
    bool data_on_patch = false;
    
    // Get the dimensions of the region.
    const hier::IntVector region_dims = region.numberCells();
    
    if (variable_name == "density")
    {
        boost::shared_ptr<pdat::CellData<double> > data_partial_density(
            BOOST_CAST<pdat::CellData<double>, hier::PatchData>(
                patch.getPatchData(d_variable_partial_density, d_plot_context)));
        
#ifdef DEBUG_CHECK_ASSERTIONS
        TBOX_ASSERT(data_partial_density);
        TBOX_ASSERT(data_partial_density->getGhostBox().isSpatiallyEqual(patch.getBox()));
#endif
        
        // Get the dimensions of box that covers the data.
        const hier::Box data_box = data_partial_density->getGhostBox();
        const hier::IntVector data_dims = data_box.numberCells();
        
        // Get the pointers to the conservative variables.
        std::vector<const double*> rho_Y;
        rho_Y.reserve(d_num_species);
        for (int si = 0; si < d_num_species; si++)
        {
            rho_Y.push_back(data_partial_density->getPointer(si));
        }
        
        size_t offset_data = data_box.offset(region.lower());
        
        if (d_dim == tbox::Dimension(1))
        {
            for (int i = 0; i < region_dims[0]; i++)
            {
                // Compute the linear indices.
                size_t idx_data = offset_data + i;
                size_t idx_region = i;
                
                std::vector<const double*> rho_Y_ptr;
                rho_Y_ptr.reserve(d_num_species);
                for (int si = 0; si < d_num_species; si++)
                {
                    rho_Y_ptr.push_back(&rho_Y[si][idx_data]);
                }
                
                buffer[idx_region] = d_equation_of_state_mixing_rules->getMixtureDensity(rho_Y_ptr);
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            for (int j = 0; j < region_dims[1]; j++)
            {
                for (int i = 0; i < region_dims[0]; i++)
                {
                    // Compute the linear indices.
                    size_t idx_data = offset_data + i +
                        j*data_dims[0];
                    
                    size_t idx_region = i +
                        j*region_dims[0];
                    
                    std::vector<const double*> rho_Y_ptr;
                    rho_Y_ptr.reserve(d_num_species);
                    for (int si = 0; si < d_num_species; si++)
                    {
                        rho_Y_ptr.push_back(&rho_Y[si][idx_data]);
                    }
                    
                    buffer[idx_region] = d_equation_of_state_mixing_rules->getMixtureDensity(rho_Y_ptr);
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            for (int k = 0; k < region_dims[2]; k++)
            {
                for (int j = 0; j < region_dims[1]; j++)
                {
                    for (int i = 0; i < region_dims[0]; i++)
                    {
                        // Compute the linear indices.
                        size_t idx_data = offset_data + i +
                            j*data_dims[0] +
                            k*data_dims[0]*data_dims[1];
                        
                        size_t idx_region = i +
                            j*region_dims[0] +
                            k*region_dims[0]*region_dims[1];
                        
                        std::vector<const double*> rho_Y_ptr;
                        rho_Y_ptr.reserve(d_num_species);
                        for (int si = 0; si < d_num_species; si++)
                        {
                            rho_Y_ptr.push_back(&rho_Y[si][idx_data]);
                        }
                        
                        buffer[idx_region] = d_equation_of_state_mixing_rules->getMixtureDensity(rho_Y_ptr);
                    }
                }
            }
        }
        
        data_on_patch = true;
    }
    else if (variable_name == "pressure")
    {
        boost::shared_ptr<pdat::CellData<double> > data_partial_density(
            BOOST_CAST<pdat::CellData<double>, hier::PatchData>(
                patch.getPatchData(d_variable_partial_density, d_plot_context)));
        
        boost::shared_ptr<pdat::CellData<double> > data_momentum(
            BOOST_CAST<pdat::CellData<double>, hier::PatchData>(
                patch.getPatchData(d_variable_momentum, d_plot_context)));
        
        boost::shared_ptr<pdat::CellData<double> > data_total_energy(
            BOOST_CAST<pdat::CellData<double>, hier::PatchData>(
                patch.getPatchData(d_variable_total_energy, d_plot_context)));
        
#ifdef DEBUG_CHECK_ASSERTIONS
        TBOX_ASSERT(data_partial_density);
        TBOX_ASSERT(data_momentum);
        TBOX_ASSERT(data_total_energy);
        TBOX_ASSERT(data_partial_density->getGhostBox().isSpatiallyEqual(patch.getBox()));
        TBOX_ASSERT(data_momentum->getGhostBox().isSpatiallyEqual(patch.getBox()));
        TBOX_ASSERT(data_total_energy->getGhostBox().isSpatiallyEqual(patch.getBox()));
#endif
        
        // Get the dimensions of box that covers the data.
        const hier::Box data_box = data_partial_density->getGhostBox();
        const hier::IntVector data_dims = data_box.numberCells();
        
        // Get the pointers to conservative variables
        std::vector<const double*> rho_Y;
        rho_Y.reserve(d_num_species);
        for (int si = 0; si < d_num_species; si++)
        {
            rho_Y.push_back(data_partial_density->getPointer(si));
        }
        const double* const rho_u = data_momentum->getPointer(0);
        const double* const rho_v = d_dim > tbox::Dimension(1) ? data_momentum->getPointer(1) : NULL;
        const double* const rho_w = d_dim > tbox::Dimension(2) ? data_momentum->getPointer(2) : NULL;
        const double* const E     = data_total_energy->getPointer(0);
        
        size_t offset_data = data_box.offset(region.lower());
        
        if (d_dim == tbox::Dimension(1))
        {
            for (int i = 0; i < region_dims[0]; i++)
            {
                // Compute the linear indices.
                size_t idx_data = offset_data + i;
                size_t idx_region = i;
                
                // Compute the mixture density.
                std::vector<const double*> rho_Y_ptr;
                rho_Y_ptr.reserve(d_num_species);
                for (int si = 0; si < d_num_species; si++)
                {
                    rho_Y_ptr.push_back(&(rho_Y[si][idx_data]));
                }
                const double rho = d_equation_of_state_mixing_rules->getMixtureDensity(
                    rho_Y_ptr);
                
                /*
                 * Compute the internal energy.
                 */
                double epsilon = 0.0;
                if (d_dim == tbox::Dimension(1))
                {
                    epsilon = (E[idx_data] -
                        0.5*rho_u[idx_data]*rho_u[idx_data]/rho)/rho;
                }
                else if (d_dim == tbox::Dimension(2))
                {
                    epsilon = (E[idx_data] -
                        0.5*(rho_u[idx_data]*rho_u[idx_data] +
                        rho_v[idx_data]*rho_v[idx_data])/rho)/rho;
                }
                else if (d_dim == tbox::Dimension(3))
                {
                    epsilon = (E[idx_data] -
                        0.5*(rho_u[idx_data]*rho_u[idx_data] +
                        rho_v[idx_data]*rho_v[idx_data] +
                        rho_w[idx_data]*rho_w[idx_data])/rho)/rho;
                }
                
                /*
                 * Compute the mass fractions.
                 */
                double Y[d_num_species];
                for (int si = 0; si < d_num_species; si++)
                {
                    Y[si] = rho_Y[si][idx_data]/rho;
                }
                
                /*
                 * Get the pointers to the mass fractions.
                 */
                std::vector<const double*> Y_ptr;
                Y_ptr.reserve(d_num_species);
                for (int si = 0; si < d_num_species; si++)
                {
                    Y_ptr.push_back(&Y[si]);
                }
                
                buffer[idx_region] = d_equation_of_state_mixing_rules->
                    getPressure(
                        &rho,
                        &epsilon,
                        Y_ptr);
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            for (int j = 0; j < region_dims[1]; j++)
            {
                for (int i = 0; i < region_dims[0]; i++)
                {
                    // Compute the linear indices.
                    size_t idx_data = offset_data + i +
                        j*data_dims[0];
                    
                    size_t idx_region = i +
                        j*region_dims[0];
                    
                    // Compute the mixture density.
                    std::vector<const double*> rho_Y_ptr;
                    rho_Y_ptr.reserve(d_num_species);
                    for (int si = 0; si < d_num_species; si++)
                    {
                        rho_Y_ptr.push_back(&(rho_Y[si][idx_data]));
                    }
                    const double rho = d_equation_of_state_mixing_rules->getMixtureDensity(
                        rho_Y_ptr);
                    
                    /*
                     * Compute the internal energy.
                     */
                    double epsilon = 0.0;
                    if (d_dim == tbox::Dimension(1))
                    {
                        epsilon = (E[idx_data] -
                            0.5*rho_u[idx_data]*rho_u[idx_data]/rho)/rho;
                    }
                    else if (d_dim == tbox::Dimension(2))
                    {
                        epsilon = (E[idx_data] -
                            0.5*(rho_u[idx_data]*rho_u[idx_data] +
                            rho_v[idx_data]*rho_v[idx_data])/rho)/rho;
                    }
                    else if (d_dim == tbox::Dimension(3))
                    {
                        epsilon = (E[idx_data] -
                            0.5*(rho_u[idx_data]*rho_u[idx_data] +
                            rho_v[idx_data]*rho_v[idx_data] +
                            rho_w[idx_data]*rho_w[idx_data])/rho)/rho;
                    }
                    
                    /*
                     * Compute the mass fractions.
                     */
                    double Y[d_num_species];
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Y[si] = rho_Y[si][idx_data]/rho;
                    }
                    
                    /*
                     * Get the pointers to the mass fractions.
                     */
                    std::vector<const double*> Y_ptr;
                    Y_ptr.reserve(d_num_species);
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Y_ptr.push_back(&Y[si]);
                    }
                    
                    buffer[idx_region] = d_equation_of_state_mixing_rules->
                        getPressure(
                            &rho,
                            &epsilon,
                            Y_ptr);
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            for (int k = 0; k < region_dims[2]; k++)
            {
                for (int j = 0; j < region_dims[1]; j++)
                {
                    for (int i = 0; i < region_dims[0]; i++)
                    {
                        // Compute the linear indices.
                        size_t idx_data = offset_data + i +
                            j*data_dims[0] +
                            k*data_dims[0]*data_dims[1];
                        
                        size_t idx_region = i +
                            j*region_dims[0] +
                            k*region_dims[0]*region_dims[1];
                        
                        // Compute the mixture density.
                        std::vector<const double*> rho_Y_ptr;
                        rho_Y_ptr.reserve(d_num_species);
                        for (int si = 0; si < d_num_species; si++)
                        {
                            rho_Y_ptr.push_back(&(rho_Y[si][idx_data]));
                        }
                        const double rho = d_equation_of_state_mixing_rules->getMixtureDensity(
                            rho_Y_ptr);
                        
                        /*
                         * Compute the internal energy.
                         */
                        double epsilon = 0.0;
                        if (d_dim == tbox::Dimension(1))
                        {
                            epsilon = (E[idx_data] -
                                0.5*rho_u[idx_data]*rho_u[idx_data]/rho)/rho;
                        }
                        else if (d_dim == tbox::Dimension(2))
                        {
                            epsilon = (E[idx_data] -
                                0.5*(rho_u[idx_data]*rho_u[idx_data] +
                                rho_v[idx_data]*rho_v[idx_data])/rho)/rho;
                        }
                        else if (d_dim == tbox::Dimension(3))
                        {
                            epsilon = (E[idx_data] -
                                0.5*(rho_u[idx_data]*rho_u[idx_data] +
                                rho_v[idx_data]*rho_v[idx_data] +
                                rho_w[idx_data]*rho_w[idx_data])/rho)/rho;
                        }
                        
                        /*
                         * Compute the mass fractions.
                         */
                        double Y[d_num_species];
                        for (int si = 0; si < d_num_species; si++)
                        {
                            Y[si] = rho_Y[si][idx_data]/rho;
                        }
                        
                        /*
                         * Get the pointers to the mass fractions.
                         */
                        std::vector<const double*> Y_ptr;
                        Y_ptr.reserve(d_num_species);
                        for (int si = 0; si < d_num_species; si++)
                        {
                            Y_ptr.push_back(&Y[si]);
                        }
                        
                        buffer[idx_region] = d_equation_of_state_mixing_rules->
                            getPressure(
                                &rho,
                                &epsilon,
                                Y_ptr);
                    }
                }
            }
        }
        
        data_on_patch = true;
    }
    else if (variable_name == "sound speed")
    {
        boost::shared_ptr<pdat::CellData<double> > data_partial_density(
            BOOST_CAST<pdat::CellData<double>, hier::PatchData>(
                patch.getPatchData(d_variable_partial_density, d_plot_context)));
        
        boost::shared_ptr<pdat::CellData<double> > data_momentum(
            BOOST_CAST<pdat::CellData<double>, hier::PatchData>(
                patch.getPatchData(d_variable_momentum, d_plot_context)));
        
        boost::shared_ptr<pdat::CellData<double> > data_total_energy(
            BOOST_CAST<pdat::CellData<double>, hier::PatchData>(
                patch.getPatchData(d_variable_total_energy, d_plot_context)));
        
#ifdef DEBUG_CHECK_ASSERTIONS
        TBOX_ASSERT(data_partial_density);
        TBOX_ASSERT(data_momentum);
        TBOX_ASSERT(data_total_energy);
        TBOX_ASSERT(data_partial_density->getGhostBox().isSpatiallyEqual(patch.getBox()));
        TBOX_ASSERT(data_momentum->getGhostBox().isSpatiallyEqual(patch.getBox()));
        TBOX_ASSERT(data_total_energy->getGhostBox().isSpatiallyEqual(patch.getBox()));
#endif
        
        // Get the dimensions of box that covers the data.
        const hier::Box data_box = data_partial_density->getGhostBox();
        const hier::IntVector data_dims = data_box.numberCells();
        
        // Get the pointers to the conservative variables.
        std::vector<const double*> rho_Y;
        rho_Y.reserve(d_num_species);
        for (int si = 0; si < d_num_species; si++)
        {
            rho_Y.push_back(data_partial_density->getPointer(si));
        }
        const double* const rho_u = data_momentum->getPointer(0);
        const double* const rho_v = d_dim > tbox::Dimension(1) ? data_momentum->getPointer(1) : NULL;
        const double* const rho_w = d_dim > tbox::Dimension(2) ? data_momentum->getPointer(2) : NULL;
        const double* const E     = data_total_energy->getPointer(0);
        
        size_t offset_data = data_box.offset(region.lower());
        
        if (d_dim == tbox::Dimension(1))
        {
            for (int i = 0; i < region_dims[0]; i++)
            {
                // Compute the linear indices.
                size_t idx_data = offset_data + i;
                size_t idx_region = i;
                
                // Compute the mixture density.
                std::vector<const double*> rho_Y_ptr;
                rho_Y_ptr.reserve(d_num_species);
                for (int si = 0; si < d_num_species; si++)
                {
                    rho_Y_ptr.push_back(&(rho_Y[si][idx_data]));
                }
                const double rho = d_equation_of_state_mixing_rules->getMixtureDensity(
                    rho_Y_ptr);
                
                /*
                 * Compute the internal energy.
                 */
                double epsilon = 0.0;
                if (d_dim == tbox::Dimension(1))
                {
                    epsilon = (E[idx_data] -
                        0.5*rho_u[idx_data]*rho_u[idx_data]/rho)/rho;
                }
                else if (d_dim == tbox::Dimension(2))
                {
                    epsilon = (E[idx_data] -
                        0.5*(rho_u[idx_data]*rho_u[idx_data] +
                        rho_v[idx_data]*rho_v[idx_data])/rho)/rho;
                }
                else if (d_dim == tbox::Dimension(3))
                {
                    epsilon = (E[idx_data] -
                        0.5*(rho_u[idx_data]*rho_u[idx_data] +
                        rho_v[idx_data]*rho_v[idx_data] +
                        rho_w[idx_data]*rho_w[idx_data])/rho)/rho;
                }
                
                /*
                 * Compute the mass fractions.
                 */
                double Y[d_num_species];
                for (int si = 0; si < d_num_species; si++)
                {
                    Y[si] = rho_Y[si][idx_data]/rho;
                }
                
                /*
                 * Get the pointers to the mass fractions.
                 */
                std::vector<const double*> Y_ptr;
                Y_ptr.reserve(d_num_species);
                for (int si = 0; si < d_num_species; si++)
                {
                    Y_ptr.push_back(&Y[si]);
                }
                
                const double p = d_equation_of_state_mixing_rules->
                    getPressure(
                        &rho,
                        &epsilon,
                        Y_ptr);
                
                buffer[idx_region] = d_equation_of_state_mixing_rules->
                    getSoundSpeed(
                        &rho,
                        &p,
                        Y_ptr);
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            for (int j = 0; j < region_dims[1]; j++)
            {
                for (int i = 0; i < region_dims[0]; i++)
                {
                    // Compute the linear indices.
                    size_t idx_data = offset_data + i +
                        j*data_dims[0];
                    
                    size_t idx_region = i +
                        j*region_dims[0];
                    
                    // Compute the mixture density.
                    std::vector<const double*> rho_Y_ptr;
                    rho_Y_ptr.reserve(d_num_species);
                    for (int si = 0; si < d_num_species; si++)
                    {
                        rho_Y_ptr.push_back(&(rho_Y[si][idx_data]));
                    }
                    const double rho = d_equation_of_state_mixing_rules->getMixtureDensity(
                        rho_Y_ptr);
                    
                    /*
                     * Compute the internal energy.
                     */
                    double epsilon = 0.0;
                    if (d_dim == tbox::Dimension(1))
                    {
                        epsilon = (E[idx_data] -
                            0.5*rho_u[idx_data]*rho_u[idx_data]/rho)/rho;
                    }
                    else if (d_dim == tbox::Dimension(2))
                    {
                        epsilon = (E[idx_data] -
                            0.5*(rho_u[idx_data]*rho_u[idx_data] +
                            rho_v[idx_data]*rho_v[idx_data])/rho)/rho;
                    }
                    else if (d_dim == tbox::Dimension(3))
                    {
                        epsilon = (E[idx_data] -
                            0.5*(rho_u[idx_data]*rho_u[idx_data] +
                            rho_v[idx_data]*rho_v[idx_data] +
                            rho_w[idx_data]*rho_w[idx_data])/rho)/rho;
                    }
                    
                    /*
                     * Compute the mass fractions.
                     */
                    double Y[d_num_species];
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Y[si] = rho_Y[si][idx_data]/rho;
                    }
                    
                    /*
                     * Get the pointers to the mass fractions.
                     */
                    std::vector<const double*> Y_ptr;
                    Y_ptr.reserve(d_num_species);
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Y_ptr.push_back(&Y[si]);
                    }
                    
                    const double p = d_equation_of_state_mixing_rules->
                        getPressure(
                            &rho,
                            &epsilon,
                            Y_ptr);
                    
                    buffer[idx_region] = d_equation_of_state_mixing_rules->
                        getSoundSpeed(
                            &rho,
                            &p,
                            Y_ptr);
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            for (int k = 0; k < region_dims[2]; k++)
            {
                for (int j = 0; j < region_dims[1]; j++)
                {
                    for (int i = 0; i < region_dims[0]; i++)
                    {
                        // Compute the linear indices.
                        size_t idx_data = offset_data + i +
                            j*data_dims[0] +
                            k*data_dims[0]*data_dims[1];
                        
                        size_t idx_region = i +
                            j*region_dims[0] +
                            k*region_dims[0]*region_dims[1];
                        
                        // Compute the mixture density.
                        std::vector<const double*> rho_Y_ptr;
                        rho_Y_ptr.reserve(d_num_species);
                        for (int si = 0; si < d_num_species; si++)
                        {
                            rho_Y_ptr.push_back(&(rho_Y[si][idx_data]));
                        }
                        const double rho = d_equation_of_state_mixing_rules->getMixtureDensity(
                            rho_Y_ptr);
                        
                        /*
                         * Compute the internal energy.
                         */
                        double epsilon = 0.0;
                        if (d_dim == tbox::Dimension(1))
                        {
                            epsilon = (E[idx_data] -
                                0.5*rho_u[idx_data]*rho_u[idx_data]/rho)/rho;
                        }
                        else if (d_dim == tbox::Dimension(2))
                        {
                            epsilon = (E[idx_data] -
                                0.5*(rho_u[idx_data]*rho_u[idx_data] +
                                rho_v[idx_data]*rho_v[idx_data])/rho)/rho;
                        }
                        else if (d_dim == tbox::Dimension(3))
                        {
                            epsilon = (E[idx_data] -
                                0.5*(rho_u[idx_data]*rho_u[idx_data] +
                                rho_v[idx_data]*rho_v[idx_data] +
                                rho_w[idx_data]*rho_w[idx_data])/rho)/rho;
                        }
                        
                        /*
                         * Compute the mass fractions.
                         */
                        double Y[d_num_species];
                        for (int si = 0; si < d_num_species; si++)
                        {
                            Y[si] = rho_Y[si][idx_data]/rho;
                        }
                        
                        /*
                         * Get the pointers to the mass fractions.
                         */
                        std::vector<const double*> Y_ptr;
                        Y_ptr.reserve(d_num_species);
                        for (int si = 0; si < d_num_species; si++)
                        {
                            Y_ptr.push_back(&Y[si]);
                        }
                        
                        const double p = d_equation_of_state_mixing_rules->
                            getPressure(
                                &rho,
                                &epsilon,
                                Y_ptr);
                        
                        buffer[idx_region] = d_equation_of_state_mixing_rules->
                            getSoundSpeed(
                                &rho,
                                &p,
                                Y_ptr);
                    }
                }
            }
        }
        
        data_on_patch = true;
    }
    else if (variable_name == "velocity")
    {
        boost::shared_ptr<pdat::CellData<double> > data_partial_density(
            BOOST_CAST<pdat::CellData<double>, hier::PatchData>(
                patch.getPatchData(d_variable_partial_density, d_plot_context)));
        
        boost::shared_ptr<pdat::CellData<double> > data_momentum(
            BOOST_CAST<pdat::CellData<double>, hier::PatchData>(
                patch.getPatchData(d_variable_momentum, d_plot_context)));
        
#ifdef DEBUG_CHECK_ASSERTIONS
        TBOX_ASSERT(data_partial_density);
        TBOX_ASSERT(data_momentum);
        TBOX_ASSERT(data_partial_density->getGhostBox().isSpatiallyEqual(patch.getBox()));
        TBOX_ASSERT(data_momentum->getGhostBox().isSpatiallyEqual(patch.getBox()));
#endif
        
        // Get the dimensions of box that covers the data.
        const hier::Box data_box = data_partial_density->getGhostBox();
        const hier::IntVector data_dims = data_box.numberCells();
        
        // Get the pointers to the conservative variables.
        std::vector<const double*> rho_Y;
        rho_Y.reserve(d_num_species);
        for (int si = 0; si < d_num_species; si++)
        {
            rho_Y.push_back(data_partial_density->getPointer(si));
        }
        const double* const m = data_momentum->getPointer(depth_id);
        
        size_t offset_data = data_box.offset(region.lower());
        
        if (d_dim == tbox::Dimension(1))
        {
            for (int i = 0; i < region_dims[0]; i++)
            {
                // Compute the linear indices.
                size_t idx_data = offset_data + i;
                size_t idx_region = i;
                
                std::vector<const double*> rho_Y_ptr;
                rho_Y_ptr.reserve(d_num_species);
                for (int si = 0; si < d_num_species; si++)
                {
                    rho_Y_ptr.push_back(&rho_Y[si][idx_data]);
                }
                
                double rho = d_equation_of_state_mixing_rules->getMixtureDensity(rho_Y_ptr);
                
                buffer[idx_region] = m[idx_data]/rho;
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            for (int j = 0; j < region_dims[1]; j++)
            {
                for (int i = 0; i < region_dims[0]; i++)
                {
                    // Compute the linear indices.
                    size_t idx_data = offset_data + i +
                        j*data_dims[0];
                    
                    size_t idx_region = i +
                        j*region_dims[0];
                    
                    std::vector<const double*> rho_Y_ptr;
                    rho_Y_ptr.reserve(d_num_species);
                    for (int si = 0; si < d_num_species; si++)
                    {
                        rho_Y_ptr.push_back(&rho_Y[si][idx_data]);
                    }
                    
                    double rho = d_equation_of_state_mixing_rules->getMixtureDensity(rho_Y_ptr);
                    
                    buffer[idx_region] = m[idx_data]/rho;
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            for (int k = 0; k < region_dims[2]; k++)
            {
                for (int j = 0; j < region_dims[1]; j++)
                {
                    for (int i = 0; i < region_dims[0]; i++)
                    {
                        // Compute the linear indices.
                        size_t idx_data = offset_data + i +
                            j*data_dims[0] +
                            k*data_dims[0]*data_dims[1];
                        
                        size_t idx_region = i +
                            j*region_dims[0] +
                            k*region_dims[0]*region_dims[1];
                        
                        std::vector<const double*> rho_Y_ptr;
                        rho_Y_ptr.reserve(d_num_species);
                        for (int si = 0; si < d_num_species; si++)
                        {
                            rho_Y_ptr.push_back(&rho_Y[si][idx_data]);
                        }
                        
                        double rho = d_equation_of_state_mixing_rules->getMixtureDensity(rho_Y_ptr);
                        
                        buffer[idx_region] = m[idx_data]/rho;
                    }
                }
            }
        }
        
        data_on_patch = true;
    }
    else if (variable_name.find("mass fraction") != std::string::npos)
    {
        int species_idx = std::stoi(variable_name.substr(14));
        
        boost::shared_ptr<pdat::CellData<double> > data_partial_density(
            BOOST_CAST<pdat::CellData<double>, hier::PatchData>(
                patch.getPatchData(d_variable_partial_density, d_plot_context)));
        
#ifdef DEBUG_CHECK_ASSERTIONS
        TBOX_ASSERT(data_partial_density);
        TBOX_ASSERT(data_partial_density->getGhostBox().isSpatiallyEqual(patch.getBox()));
#endif
        
        // Get the dimensions of box that covers the data.
        const hier::Box data_box = data_partial_density->getGhostBox();
        const hier::IntVector data_dims = data_box.numberCells();
        
        // Get the pointers to conservative variables.
        std::vector<const double*> rho_Y;
        rho_Y.reserve(d_num_species);
        for (int si = 0; si < d_num_species; si++)
        {
            rho_Y.push_back(data_partial_density->getPointer(si));
        }
        
        size_t offset_data = data_box.offset(region.lower());
        
        if (d_dim == tbox::Dimension(1))
        {
            for (int i = 0; i < region_dims[0]; i++)
            {
                // Compute the linear indices.
                size_t idx_data = offset_data + i;
                
                size_t idx_region = i;
                
                std::vector<const double*> rho_Y_ptr;
                rho_Y_ptr.reserve(d_num_species);
                for (int si = 0; si < d_num_species; si++)
                {
                    rho_Y_ptr.push_back(&rho_Y[si][idx_data]);
                }
                
                double rho = d_equation_of_state_mixing_rules->getMixtureDensity(rho_Y_ptr);
                
                buffer[idx_region] = rho_Y[species_idx][idx_data]/rho;
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            for (int j = 0; j < region_dims[1]; j++)
            {
                for (int i = 0; i < region_dims[0]; i++)
                {
                    // Compute the linear indices.
                    size_t idx_data = offset_data + i +
                        j*data_dims[0];
                    
                    size_t idx_region = i +
                        j*region_dims[0];
                    
                    std::vector<const double*> rho_Y_ptr;
                    rho_Y_ptr.reserve(d_num_species);
                    for (int si = 0; si < d_num_species; si++)
                    {
                        rho_Y_ptr.push_back(&rho_Y[si][idx_data]);
                    }
                    
                    double rho = d_equation_of_state_mixing_rules->getMixtureDensity(rho_Y_ptr);
                    
                    buffer[idx_region] = rho_Y[species_idx][idx_data]/rho;
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            for (int k = 0; k < region_dims[2]; k++)
            {
                for (int j = 0; j < region_dims[1]; j++)
                {
                    for (int i = 0; i < region_dims[0]; i++)
                    {
                        // Compute the linear indices.
                        size_t idx_data = offset_data + i +
                            j*data_dims[0] +
                            k*data_dims[0]*data_dims[1];
                        
                        size_t idx_region = i +
                            j*region_dims[0] +
                            k*region_dims[0]*region_dims[1];
                        
                        std::vector<const double*> rho_Y_ptr;
                        rho_Y_ptr.reserve(d_num_species);
                        for (int si = 0; si < d_num_species; si++)
                        {
                            rho_Y_ptr.push_back(&rho_Y[si][idx_data]);
                        }
                        
                        double rho = d_equation_of_state_mixing_rules->getMixtureDensity(rho_Y_ptr);
                        
                        buffer[idx_region] = rho_Y[species_idx][idx_data]/rho;
                    }
                }
            }
        }
        
        data_on_patch = true;
    }
    else
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::"
            << "packDerivedDataIntoDoubleBuffer()\n"
            << "Unknown/unsupported variable '"
            << variable_name
            << "' required to compute."
            << std::endl);
    }
    
    return data_on_patch;
}


/*
 * Register the plotting quantities.
 */
#ifdef HAVE_HDF5
void
FlowModelFourEqnConservative::registerPlotQuantities(
    const boost::shared_ptr<appu::VisItDataWriter>& visit_writer)
{
    if (!d_plot_context)
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::"
            << "registerPlotQuantities()\n"
            << "The plotting context is not set yet."
            << std::endl);
    }
    
    hier::VariableDatabase* vardb = hier::VariableDatabase::getDatabase();
    
    for (int si = 0; si < d_num_species; si++)
    {
        std::string partial_density_name =
            "partial density " + tbox::Utilities::intToString(si);
        
        visit_writer->registerPlotQuantity(
            partial_density_name,
            "SCALAR",
            vardb->mapVariableAndContextToIndex(
                d_variable_partial_density,
                d_plot_context),
            si);
    }
    
    visit_writer->registerPlotQuantity(
        "momentum",
        "VECTOR",
        vardb->mapVariableAndContextToIndex(
           d_variable_momentum,
           d_plot_context));
    
    visit_writer->registerPlotQuantity(
        "total energy",
        "SCALAR",
        vardb->mapVariableAndContextToIndex(
           d_variable_total_energy,
           d_plot_context));
    
    visit_writer->registerDerivedPlotQuantity("pressure",
        "SCALAR",
        this);
    
    visit_writer->registerDerivedPlotQuantity("sound speed",
        "SCALAR",
        this);
    
    visit_writer->registerDerivedPlotQuantity("velocity",
        "VECTOR",
        this);
    
    visit_writer->registerDerivedPlotQuantity("density",
        "SCALAR",
        this);
    
    for (int si = 0; si < d_num_species; si++)
    {
        std::string mass_fraction_name =
            "mass fraction " + tbox::Utilities::intToString(si);
            
        visit_writer->registerDerivedPlotQuantity(mass_fraction_name,
            "SCALAR",
            this);
    }
}
#endif


/*
 * Set the number of sub-ghost cells of a variable.
 * This function can be called recursively if the variables are computed recursively.
 */
void
FlowModelFourEqnConservative::setNumberOfSubGhosts(
    const hier::IntVector& num_subghosts,
    const std::string& variable_name,
    const std::string& parent_variable_name)
{
    if (variable_name == "DENSITY")
    {
        if (d_num_subghosts_density > -hier::IntVector::getOne(d_dim))
        {
            if (num_subghosts > d_num_subghosts_density)
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::setNumberOfSubGhosts()\n"
                    << "Number of ghosts of '"
                    << parent_variable_name
                    << "' exceeds"
                    << " number of ghosts of '"
                    << variable_name
                    << "'."
                    << std::endl);
            }
        }
        else
        {
            d_num_subghosts_density = num_subghosts;
        }
    }
    else if (variable_name == "MASS_FRACTION")
    {
        if (d_num_subghosts_mass_fraction > -hier::IntVector::getOne(d_dim))
        {
            if (num_subghosts > d_num_subghosts_mass_fraction)
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::setNumberOfSubGhosts()\n"
                    << "Number of ghosts of '"
                    << parent_variable_name
                    << "' exceeds"
                    << " number of ghosts of '"
                    << variable_name
                    << "'."
                    << std::endl);
            }
        }
        else
        {
            d_num_subghosts_mass_fraction = num_subghosts;
        }
        
        setNumberOfSubGhosts(num_subghosts, "DENSITY", parent_variable_name);
    }
    else if (variable_name == "VELOCITY")
    {
        if (d_num_subghosts_velocity > -hier::IntVector::getOne(d_dim))
        {
            if (num_subghosts > d_num_subghosts_velocity)
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::setNumberOfSubGhosts()\n"
                    << "Number of ghosts of '"
                    << parent_variable_name
                    << "' exceeds"
                    << " number of ghosts of '"
                    << variable_name
                    << "'."
                    << std::endl);
            }
        }
        else
        {
            d_num_subghosts_velocity = num_subghosts;
        }
        
        setNumberOfSubGhosts(num_subghosts, "DENSITY", parent_variable_name);
    }
    else if (variable_name == "INTERNAL_ENERGY")
    {
        if (d_num_subghosts_internal_energy > -hier::IntVector::getOne(d_dim))
        {
            if (num_subghosts > d_num_subghosts_internal_energy)
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::setNumberOfSubGhosts()\n"
                    << "Number of ghosts of '"
                    << parent_variable_name
                    << "' exceeds"
                    << " number of ghosts of '"
                    << variable_name
                    << "'."
                    << std::endl);
            }
        }
        else
        {
            d_num_subghosts_internal_energy = num_subghosts;
        }
        
        setNumberOfSubGhosts(num_subghosts, "DENSITY", parent_variable_name);
        setNumberOfSubGhosts(num_subghosts, "VELOCITY", parent_variable_name);
    }
    else if (variable_name == "PRESSURE")
    {
        if (d_num_subghosts_pressure > -hier::IntVector::getOne(d_dim))
        {
            if (num_subghosts > d_num_subghosts_pressure)
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::setNumberOfSubGhosts()\n"
                    << "Number of ghosts of '"
                    << parent_variable_name
                    << "' exceeds"
                    << " number of ghosts of '"
                    << variable_name
                    << "'."
                    << std::endl);
            }
        }
        else
        {
            d_num_subghosts_pressure = num_subghosts;
        }
        
        setNumberOfSubGhosts(num_subghosts, "DENSITY", parent_variable_name);
        setNumberOfSubGhosts(num_subghosts, "MASS_FRACTION", parent_variable_name);
        setNumberOfSubGhosts(num_subghosts, "INTERNAL_ENERGY", parent_variable_name);
    }
    else if (variable_name == "SOUND_SPEED")
    {
        if (d_num_subghosts_sound_speed > -hier::IntVector::getOne(d_dim))
        {
            if (num_subghosts > d_num_subghosts_sound_speed)
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::setNumberOfSubGhosts()\n"
                    << "Number of ghosts of '"
                    << parent_variable_name
                    << "' exceeds"
                    << " number of ghosts of '"
                    << variable_name
                    << "'."
                    << std::endl);
            }
        }
        else
        {
            d_num_subghosts_sound_speed = num_subghosts;
        }
        
        setNumberOfSubGhosts(num_subghosts, "DENSITY", parent_variable_name);
        setNumberOfSubGhosts(num_subghosts, "MASS_FRACTION", parent_variable_name);
        setNumberOfSubGhosts(num_subghosts, "PRESSURE", parent_variable_name);
    }
    else if (variable_name == "TEMPERATURE")
    {
        if (d_num_subghosts_temperature > -hier::IntVector::getOne(d_dim))
        {
            if (num_subghosts > d_num_subghosts_temperature)
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::setNumberOfSubGhosts()\n"
                    << "Number of ghosts of '"
                    << parent_variable_name
                    << "' exceeds"
                    << " number of ghosts of '"
                    << variable_name
                    << "'."
                    << std::endl);
            }
        }
        else
        {
            d_num_subghosts_temperature = num_subghosts;
        }
        
        setNumberOfSubGhosts(num_subghosts, "DENSITY", parent_variable_name);
        setNumberOfSubGhosts(num_subghosts, "MASS_FRACTION", parent_variable_name);
        setNumberOfSubGhosts(num_subghosts, "PRESSURE", parent_variable_name);
    }
    else if (variable_name == "DILATATION")
    {
        if (d_num_subghosts_dilatation > -hier::IntVector::getOne(d_dim))
        {
            if (num_subghosts > d_num_subghosts_dilatation)
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::setNumberOfSubGhosts()\n"
                    << "Number of ghosts of '"
                    << parent_variable_name
                    << "' exceeds"
                    << " number of ghosts of '"
                    << variable_name
                    << "'."
                    << std::endl);
            }
        }
        else
        {
            d_num_subghosts_dilatation = num_subghosts;
        }
        
        setNumberOfSubGhosts(num_subghosts, "DENSITY", parent_variable_name);
        setNumberOfSubGhosts(num_subghosts, "VELOCITY", parent_variable_name);
    }
    else if (variable_name == "VORTICITY")
    {
        if (d_num_subghosts_vorticity > -hier::IntVector::getOne(d_dim))
        {
            if (num_subghosts > d_num_subghosts_vorticity)
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::setNumberOfSubGhosts()\n"
                    << "Number of ghosts of '"
                    << parent_variable_name
                    << "' exceeds"
                    << " number of ghosts of '"
                    << variable_name
                    << "'."
                    << std::endl);
            }
        }
        else
        {
            d_num_subghosts_vorticity = num_subghosts;
        }
        
        setNumberOfSubGhosts(num_subghosts, "DENSITY", parent_variable_name);
        setNumberOfSubGhosts(num_subghosts, "VELOCITY", parent_variable_name);
    }
    else if (variable_name == "ENSTROPHY")
    {
        if (d_num_subghosts_enstrophy > -hier::IntVector::getOne(d_dim))
        {
            if (num_subghosts > d_num_subghosts_enstrophy)
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::setNumberOfSubGhosts()\n"
                    << "Number of ghosts of '"
                    << parent_variable_name
                    << "' exceeds"
                    << " number of ghosts of '"
                    << variable_name
                    << "'."
                    << std::endl);
            }
        }
        else
        {
            d_num_subghosts_enstrophy = num_subghosts;
        }
        
        setNumberOfSubGhosts(num_subghosts, "VORTICITY", parent_variable_name);
    }
    else if (variable_name == "CONVECTIVE_FLUX_X")
    {
        if (d_num_subghosts_convective_flux_x > -hier::IntVector::getOne(d_dim))
        {
            if (num_subghosts > d_num_subghosts_convective_flux_x)
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::setNumberOfSubGhosts()\n"
                    << "Number of ghosts of '"
                    << parent_variable_name
                    << "' exceeds"
                    << " number of ghosts of '"
                    << variable_name
                    << "'."
                    << std::endl);
            }
        }
        else
        {
            d_num_subghosts_convective_flux_x = num_subghosts;
        }
        
        setNumberOfSubGhosts(num_subghosts, "VELOCITY", parent_variable_name);
        setNumberOfSubGhosts(num_subghosts, "PRESSURE", parent_variable_name);
    }
    else if (variable_name == "CONVECTIVE_FLUX_Y")
    {
        if (d_num_subghosts_convective_flux_y > -hier::IntVector::getOne(d_dim))
        {
            if (num_subghosts > d_num_subghosts_convective_flux_y)
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::setNumberOfSubGhosts()\n"
                    << "Number of ghosts of '"
                    << parent_variable_name
                    << "' exceeds"
                    << " number of ghosts of '"
                    << variable_name
                    << "'."
                    << std::endl);
            }
        }
        else
        {
            d_num_subghosts_convective_flux_y = num_subghosts;
        }
        
        setNumberOfSubGhosts(num_subghosts, "VELOCITY", parent_variable_name);
        setNumberOfSubGhosts(num_subghosts, "PRESSURE", parent_variable_name);
    }
    else if (variable_name == "CONVECTIVE_FLUX_Z")
    {
        if (d_num_subghosts_convective_flux_z > -hier::IntVector::getOne(d_dim))
        {
            if (num_subghosts > d_num_subghosts_convective_flux_z)
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::setNumberOfSubGhosts()\n"
                    << "Number of ghosts of '"
                    << parent_variable_name
                    << "' exceeds"
                    << " number of ghosts of '"
                    << variable_name
                    << "'."
                    << std::endl);
            }
        }
        else
        {
            d_num_subghosts_convective_flux_z = num_subghosts;
        }
        
        setNumberOfSubGhosts(num_subghosts, "VELOCITY", parent_variable_name);
        setNumberOfSubGhosts(num_subghosts, "PRESSURE", parent_variable_name);
    }
    else if (variable_name == "PRIMITIVE_VARIABLES")
    {
        setNumberOfSubGhosts(num_subghosts, "VELOCITY", parent_variable_name);
        setNumberOfSubGhosts(num_subghosts, "PRESSURE", parent_variable_name);
    }
    else if (variable_name == "MAX_WAVE_SPEED_X")
    {
        if (d_num_subghosts_max_wave_speed_x > -hier::IntVector::getOne(d_dim))
        {
            if (num_subghosts > d_num_subghosts_max_wave_speed_x)
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::setNumberOfSubGhosts()\n"
                    << "Number of ghosts of '"
                    << parent_variable_name
                    << "' exceeds"
                    << " number of ghosts of '"
                    << variable_name
                    << "'."
                    << std::endl);
            }
        }
        else
        {
            d_num_subghosts_max_wave_speed_x = num_subghosts;
        }
        
        setNumberOfSubGhosts(num_subghosts, "VELOCITY", parent_variable_name);
        setNumberOfSubGhosts(num_subghosts, "SOUND_SPEED", parent_variable_name);
    }
    else if (variable_name == "MAX_WAVE_SPEED_Y")
    {
        if (d_num_subghosts_max_wave_speed_y > -hier::IntVector::getOne(d_dim))
        {
            if (num_subghosts > d_num_subghosts_max_wave_speed_y)
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::setNumberOfSubGhosts()\n"
                    << "Number of ghosts of '"
                    << parent_variable_name
                    << "' exceeds"
                    << " number of ghosts of '"
                    << variable_name
                    << "'."
                    << std::endl);
            }
        }
        else
        {
            d_num_subghosts_max_wave_speed_y = num_subghosts;
        }
        
        setNumberOfSubGhosts(num_subghosts, "VELOCITY", parent_variable_name);
        setNumberOfSubGhosts(num_subghosts, "SOUND_SPEED", parent_variable_name);
    }
    else if (variable_name == "MAX_WAVE_SPEED_Z")
    {
        if (d_num_subghosts_max_wave_speed_z > -hier::IntVector::getOne(d_dim))
        {
            if (num_subghosts > d_num_subghosts_max_wave_speed_z)
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::setNumberOfSubGhosts()\n"
                    << "Number of ghosts of '"
                    << parent_variable_name
                    << "' exceeds"
                    << " number of ghosts of '"
                    << variable_name
                    << "'."
                    << std::endl);
            }
        }
        else
        {
            d_num_subghosts_max_wave_speed_z = num_subghosts;
        }
        
        setNumberOfSubGhosts(num_subghosts, "VELOCITY", parent_variable_name);
        setNumberOfSubGhosts(num_subghosts, "SOUND_SPEED", parent_variable_name);
    }
}


/*
 * Set the ghost boxes and their dimensions of derived cell variables.
 */
void
FlowModelFourEqnConservative::setGhostBoxesAndDimensionsDerivedCellVariables()
{
    if (d_num_subghosts_density > -hier::IntVector::getOne(d_dim))
    {
        d_subghost_box_density = d_interior_box;
        d_subghost_box_density.grow(d_num_subghosts_density);
        d_subghostcell_dims_density = d_subghost_box_density.numberCells();
    }
    
    if (d_num_subghosts_mass_fraction > -hier::IntVector::getOne(d_dim))
    {
        d_subghost_box_mass_fraction = d_interior_box;
        d_subghost_box_mass_fraction.grow(d_num_subghosts_mass_fraction);
        d_subghostcell_dims_mass_fraction = d_subghost_box_mass_fraction.numberCells();
    }
    
    if (d_num_subghosts_velocity > -hier::IntVector::getOne(d_dim))
    {
        d_subghost_box_velocity = d_interior_box;
        d_subghost_box_velocity.grow(d_num_subghosts_velocity);
        d_subghostcell_dims_velocity = d_subghost_box_velocity.numberCells();
    }
    
    if (d_num_subghosts_internal_energy > -hier::IntVector::getOne(d_dim))
    {
        d_subghost_box_internal_energy = d_interior_box;
        d_subghost_box_internal_energy.grow(d_num_subghosts_internal_energy);
        d_subghostcell_dims_internal_energy = d_subghost_box_internal_energy.numberCells();
    }
    
    if (d_num_subghosts_pressure > -hier::IntVector::getOne(d_dim))
    {
        d_subghost_box_pressure = d_interior_box;
        d_subghost_box_pressure.grow(d_num_subghosts_pressure);
        d_subghostcell_dims_pressure = d_subghost_box_pressure.numberCells();
    }
    
    if (d_num_subghosts_sound_speed > -hier::IntVector::getOne(d_dim))
    {
        d_subghost_box_sound_speed = d_interior_box;
        d_subghost_box_sound_speed.grow(d_num_subghosts_sound_speed);
        d_subghostcell_dims_sound_speed = d_subghost_box_sound_speed.numberCells();
    }
    
    if (d_num_subghosts_temperature > -hier::IntVector::getOne(d_dim))
    {
        d_subghost_box_temperature = d_interior_box;
        d_subghost_box_temperature.grow(d_num_subghosts_temperature);
        d_subghostcell_dims_temperature = d_subghost_box_temperature.numberCells();
    }
    
    if (d_num_subghosts_dilatation > -hier::IntVector::getOne(d_dim))
    {
        d_subghost_box_dilatation = d_interior_box;
        d_subghost_box_dilatation.grow(d_num_subghosts_dilatation);
        d_subghostcell_dims_dilatation = d_subghost_box_dilatation.numberCells();
    }
    
    if (d_num_subghosts_vorticity > -hier::IntVector::getOne(d_dim))
    {
        d_subghost_box_vorticity = d_interior_box;
        d_subghost_box_vorticity.grow(d_num_subghosts_vorticity);
        d_subghostcell_dims_vorticity = d_subghost_box_vorticity.numberCells();
    }
    
    if (d_num_subghosts_enstrophy > -hier::IntVector::getOne(d_dim))
    {
        d_subghost_box_enstrophy = d_interior_box;
        d_subghost_box_enstrophy.grow(d_num_subghosts_enstrophy);
        d_subghostcell_dims_enstrophy = d_subghost_box_enstrophy.numberCells();
    }
    
    if (d_num_subghosts_convective_flux_x > -hier::IntVector::getOne(d_dim))
    {
        d_subghost_box_convective_flux_x = d_interior_box;
        d_subghost_box_convective_flux_x.grow(d_num_subghosts_convective_flux_x);
        d_subghostcell_dims_convective_flux_x = d_subghost_box_convective_flux_x.numberCells();
    }
    
    if (d_num_subghosts_convective_flux_y > -hier::IntVector::getOne(d_dim))
    {
        d_subghost_box_convective_flux_y = d_interior_box;
        d_subghost_box_convective_flux_y.grow(d_num_subghosts_convective_flux_y);
        d_subghostcell_dims_convective_flux_y = d_subghost_box_convective_flux_y.numberCells();
    }
    
    if (d_num_subghosts_convective_flux_z > -hier::IntVector::getOne(d_dim))
    {
        d_subghost_box_convective_flux_z = d_interior_box;
        d_subghost_box_convective_flux_z.grow(d_num_subghosts_convective_flux_z);
        d_subghostcell_dims_convective_flux_z = d_subghost_box_convective_flux_z.numberCells();
    }
    
    if (d_num_subghosts_max_wave_speed_x > -hier::IntVector::getOne(d_dim))
    {
        d_subghost_box_max_wave_speed_x = d_interior_box;
        d_subghost_box_max_wave_speed_x.grow(d_num_subghosts_max_wave_speed_x);
        d_subghostcell_dims_max_wave_speed_x = d_subghost_box_max_wave_speed_x.numberCells();
    }
    
    if (d_num_subghosts_max_wave_speed_y > -hier::IntVector::getOne(d_dim))
    {
        d_subghost_box_max_wave_speed_y = d_interior_box;
        d_subghost_box_max_wave_speed_y.grow(d_num_subghosts_max_wave_speed_y);
        d_subghostcell_dims_max_wave_speed_y = d_subghost_box_max_wave_speed_y.numberCells();
    }
    
    if (d_num_subghosts_max_wave_speed_z > -hier::IntVector::getOne(d_dim))
    {
        d_subghost_box_max_wave_speed_z = d_interior_box;
        d_subghost_box_max_wave_speed_z.grow(d_num_subghosts_max_wave_speed_z);
        d_subghostcell_dims_max_wave_speed_z = d_subghost_box_max_wave_speed_z.numberCells();
    }
    
    if (d_num_subghosts_diffusivities > -hier::IntVector::getOne(d_dim))
    {
        d_subghost_box_diffusivities = d_interior_box;
        d_subghost_box_diffusivities.grow(d_num_subghosts_diffusivities);
        d_subghostcell_dims_diffusivities = d_subghost_box_diffusivities.numberCells();
    }
}


/*
 * Get the global cell data of partial density in the registered patch.
 */
boost::shared_ptr<pdat::CellData<double> >
FlowModelFourEqnConservative::getGlobalCellDataPartialDensity()
{
    // Get the cell data of the registered variable partial density.
    boost::shared_ptr<pdat::CellData<double> > data_partial_density(
        BOOST_CAST<pdat::CellData<double>, hier::PatchData>(
            d_patch->getPatchData(d_variable_partial_density, getDataContext())));
    
    return data_partial_density;
}


/*
 * Get the global cell data of momentum in the registered patch.
 */
boost::shared_ptr<pdat::CellData<double> >
FlowModelFourEqnConservative::getGlobalCellDataMomentum()
{
    // Get the cell data of the registered variable momentum.
    boost::shared_ptr<pdat::CellData<double> > data_momentum(
        BOOST_CAST<pdat::CellData<double>, hier::PatchData>(
            d_patch->getPatchData(d_variable_momentum, getDataContext())));
    
    return data_momentum;
}


/*
 * Get the global cell data of total energy in the registered patch.
 */
boost::shared_ptr<pdat::CellData<double> >
FlowModelFourEqnConservative::getGlobalCellDataTotalEnergy()
{
    // Get the cell data of the registered variable total energy.
    boost::shared_ptr<pdat::CellData<double> > data_total_energy(
        BOOST_CAST<pdat::CellData<double>, hier::PatchData>(
            d_patch->getPatchData(d_variable_total_energy, getDataContext())));
    
    return data_total_energy;
}


/*
 * Compute the global cell data of density in the registered patch.
 */
void
FlowModelFourEqnConservative::computeGlobalCellDataDensity()
{
    if (d_num_subghosts_density > -hier::IntVector::getOne(d_dim))
    {
        // Create the cell data of density.
        d_data_density.reset(
            new pdat::CellData<double>(d_interior_box, 1, d_num_subghosts_density));
        
        // Get the cell data of the variable partial density.
        boost::shared_ptr<pdat::CellData<double> > data_partial_density =
            getGlobalCellDataPartialDensity();
        
        // Get the pointers to the cell data of denisty and partial density.
        double* rho = d_data_density->getPointer(0);
        std::vector<double*> rho_Y;
        rho_Y.reserve(d_num_species);
        for (int si = 0; si < d_num_species; si++)
        {
            rho_Y.push_back(data_partial_density->getPointer(si));
        }
        
        if (d_dim == tbox::Dimension(1))
        {
            // Compute the density field.
            for (int i = -d_num_subghosts_density[0];
                 i < d_interior_dims[0] + d_num_subghosts_density[0];
                 i++)
            {
                // Compute the linear indices.
                const int idx = i + d_num_ghosts[0];
                const int idx_density = i + d_num_subghosts_density[0];
                
                std::vector<const double*> rho_Y_ptr;
                rho_Y_ptr.reserve(d_num_species);
                for (int si = 0; si < d_num_species; si++)
                {
                    rho_Y_ptr.push_back(&rho_Y[si][idx]);
                }
                
                rho[idx_density] = d_equation_of_state_mixing_rules->
                    getMixtureDensity(
                        rho_Y_ptr);
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            // Compute the density field.
            for (int j = -d_num_subghosts_density[1];
                 j < d_interior_dims[1] + d_num_subghosts_density[1];
                 j++)
            {
                for (int i = -d_num_subghosts_density[0];
                     i < d_interior_dims[0] + d_num_subghosts_density[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx = (i + d_num_ghosts[0]) +
                        (j + d_num_ghosts[1])*d_ghostcell_dims[0];
                    
                    const int idx_density = (i + d_num_subghosts_density[0]) +
                        (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0];
                    
                    std::vector<const double*> rho_Y_ptr;
                    rho_Y_ptr.reserve(d_num_species);
                    for (int si = 0; si < d_num_species; si++)
                    {
                        rho_Y_ptr.push_back(&rho_Y[si][idx]);
                    }
                    
                    rho[idx_density] = d_equation_of_state_mixing_rules->
                        getMixtureDensity(
                            rho_Y_ptr);
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            // Compute the density field.
            for (int k = -d_num_subghosts_density[2];
                 k < d_interior_dims[2] + d_num_subghosts_density[2];
                 k++)
            {
                for (int j = -d_num_subghosts_density[1];
                     j < d_interior_dims[1] + d_num_subghosts_density[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_density[0];
                         i < d_interior_dims[0] + d_num_subghosts_density[0];
                         i++)
                    {
                        // Compute the linear indices.
                        const int idx = (i + d_num_ghosts[0]) +
                            (j + d_num_ghosts[1])*d_ghostcell_dims[0] +
                            (k + d_num_ghosts[2])*d_ghostcell_dims[0]*d_ghostcell_dims[1];
                        
                        const int idx_density = (i + d_num_subghosts_density[0]) +
                            (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0] +
                            (k + d_num_subghosts_density[2])*d_subghostcell_dims_density[0]*d_subghostcell_dims_density[1];
                        
                        std::vector<const double*> rho_Y_ptr;
                        rho_Y_ptr.reserve(d_num_species);
                        for (int si = 0; si < d_num_species; si++)
                        {
                            rho_Y_ptr.push_back(&rho_Y[si][idx]);
                        }
                        
                        rho[idx_density] = d_equation_of_state_mixing_rules->
                            getMixtureDensity(
                                rho_Y_ptr);
                    }
                }
            }
        }
    }
    else
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::computeGlobalCellDataDensity()\n"
            << "Cell data of 'DENSITY' is not yet registered."
            << std::endl);
    }
}


/*
 * Compute the global cell data of mass fraction with density in the registered patch.
 */
void
FlowModelFourEqnConservative::computeGlobalCellDataMassFractionWithDensity()
{
    if (d_num_subghosts_mass_fraction > -hier::IntVector::getOne(d_dim))
    {
        // Create the cell data of mass fraction.
        d_data_mass_fraction.reset(
            new pdat::CellData<double>(d_interior_box, d_num_species, d_num_subghosts_mass_fraction));
        
        // Get the cell data of the variable partial density.
        boost::shared_ptr<pdat::CellData<double> > data_partial_density =
            getGlobalCellDataPartialDensity();
        
        if (!d_data_density)
        {
            computeGlobalCellDataDensity();
        }
        
        // Get the pointers to the cell data of mass fraction, denisty and partial density.
        std::vector<double*> Y;
        Y.reserve(d_num_species);
        for (int si = 0; si < d_num_species; si++)
        {
            Y.push_back(d_data_mass_fraction->getPointer(si));
        }
        double* rho = d_data_density->getPointer(0);
        std::vector<double*> rho_Y;
        rho_Y.reserve(d_num_species);
        for (int si = 0; si < d_num_species; si++)
        {
            rho_Y.push_back(data_partial_density->getPointer(si));
        }
        
        if (d_dim == tbox::Dimension(1))
        {
            // Compute the mass fraction field.
            for (int i = -d_num_subghosts_mass_fraction[0];
                 i < d_interior_dims[0] + d_num_subghosts_mass_fraction[0];
                 i++)
            {
                // Compute the linear indices.
                const int idx = i + d_num_ghosts[0];
                const int idx_density = i + d_num_subghosts_density[0];
                const int idx_mass_fraction = i + d_num_subghosts_mass_fraction[0];
                
                for (int si = 0; si < d_num_species; si++)
                {
                    Y[si][idx_mass_fraction] = rho_Y[si][idx]/rho[idx_density];
                }
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            // Compute the mass fraction field.
            for (int j = -d_num_subghosts_mass_fraction[1];
                 j < d_interior_dims[1] + d_num_subghosts_mass_fraction[1];
                 j++)
            {
                for (int i = -d_num_subghosts_mass_fraction[0];
                     i < d_interior_dims[0] + d_num_subghosts_mass_fraction[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx = (i + d_num_ghosts[0]) +
                        (j + d_num_ghosts[1])*d_ghostcell_dims[0];
                    
                    const int idx_density = (i + d_num_subghosts_density[0]) +
                        (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0];
                    
                    const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                        (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0];
                    
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Y[si][idx_mass_fraction] = rho_Y[si][idx]/rho[idx_density];
                    }
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            // Compute the mass fraction field.
            for (int k = -d_num_subghosts_mass_fraction[2];
                 k < d_interior_dims[2] + d_num_subghosts_mass_fraction[2];
                 k++)
            {
                for (int j = -d_num_subghosts_mass_fraction[1];
                     j < d_interior_dims[1] + d_num_subghosts_mass_fraction[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_mass_fraction[0];
                         i < d_interior_dims[0] + d_num_subghosts_mass_fraction[0];
                         i++)
                    {
                        // Compute the linear indices.
                        const int idx = (i + d_num_ghosts[0]) +
                            (j + d_num_ghosts[1])*d_ghostcell_dims[0] +
                            (k + d_num_ghosts[2])*d_ghostcell_dims[0]*d_ghostcell_dims[1];
                        
                        const int idx_density = (i + d_num_subghosts_density[0]) +
                            (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0] +
                            (k + d_num_subghosts_density[2])*d_subghostcell_dims_density[0]*
                                d_subghostcell_dims_density[1];
                        
                        const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                            (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0] +
                            (k + d_num_subghosts_mass_fraction[2])*d_subghostcell_dims_mass_fraction[0]*
                                d_subghostcell_dims_mass_fraction[1];
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            Y[si][idx_mass_fraction] = rho_Y[si][idx]/rho[idx_density];
                        }
                    }
                }
            }
        }
    }
    else
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::computeGlobalCellDataMassFractionWithDensity()\n"
            << "Cell data of 'MASS_FRACTION' is not yet registered."
            << std::endl);
    }
}


/*
 * Compute the global cell data of velocity with density in the registered patch.
 */
void
FlowModelFourEqnConservative::computeGlobalCellDataVelocityWithDensity()
{
    if (d_num_subghosts_velocity > -hier::IntVector::getOne(d_dim))
    {
        // Create the cell data of velocity.
        d_data_velocity.reset(
            new pdat::CellData<double>(d_interior_box, d_dim.getValue(), d_num_subghosts_velocity));
        
        // Get the cell data of the variable momentum.
        boost::shared_ptr<pdat::CellData<double> > data_momentum =
            getGlobalCellDataMomentum();
        
        if (!d_data_density)
        {
            computeGlobalCellDataDensity();
        }
        
        // Get the pointer to the cell data of density.
        double* rho = d_data_density->getPointer(0);
        
        if (d_dim == tbox::Dimension(1))
        {
            // Get the pointer to the cell data of velocity.
            double* u = d_data_velocity->getPointer(0);
            
            // Get the pointer to the cell data of momentum.
            double* rho_u = data_momentum->getPointer(0);
            
            // Compute the velocity field.
            for (int i = -d_num_subghosts_velocity[0];
                 i < d_interior_dims[0] + d_num_subghosts_velocity[0];
                 i++)
            {
                // Compute the linear indices.
                const int idx = i + d_num_ghosts[0];
                const int idx_density = i + d_num_subghosts_density[0];
                const int idx_velocity = i + d_num_subghosts_velocity[0];
                
                u[idx_velocity] = rho_u[idx]/rho[idx_density];
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            // Get the pointers to the cell data of velocity.
            double* u = d_data_velocity->getPointer(0);
            double* v = d_data_velocity->getPointer(1);
            
            // Get the pointers to the cell data of momentum.
            double* rho_u = data_momentum->getPointer(0);
            double* rho_v = data_momentum->getPointer(1);
            
            // Compute the velocity field.
            for (int j = -d_num_subghosts_velocity[1];
                 j < d_interior_dims[1] + d_num_subghosts_velocity[1];
                 j++)
            {
                for (int i = -d_num_subghosts_velocity[0];
                     i < d_interior_dims[0] + d_num_subghosts_velocity[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx = (i + d_num_ghosts[0]) +
                        (j + d_num_ghosts[1])*d_ghostcell_dims[0];
                    
                    const int idx_density = (i + d_num_subghosts_density[0]) +
                        (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0];
                    
                    const int idx_velocity = (i + d_num_subghosts_velocity[0]) +
                        (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                    
                    u[idx_velocity] = rho_u[idx]/rho[idx_density];
                    v[idx_velocity] = rho_v[idx]/rho[idx_density];
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            // Get the pointers to the cell data of velocity.
            double* u = d_data_velocity->getPointer(0);
            double* v = d_data_velocity->getPointer(1);
            double* w = d_data_velocity->getPointer(2);
            
            // Get the pointers to the cell data of momentum.
            double* rho_u = data_momentum->getPointer(0);
            double* rho_v = data_momentum->getPointer(1);
            double* rho_w = data_momentum->getPointer(2);
            
            // Compute the velocity field.
            for (int k = -d_num_subghosts_velocity[2];
                 k < d_interior_dims[2] + d_num_subghosts_velocity[2];
                 k++)
            {
                for (int j = -d_num_subghosts_velocity[1];
                     j < d_interior_dims[1] + d_num_subghosts_velocity[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_velocity[0];
                         i < d_interior_dims[0] + d_num_subghosts_velocity[0];
                         i++)
                    {
                        // Compute the linear indices.
                        const int idx = (i + d_num_ghosts[0]) +
                            (j + d_num_ghosts[1])*d_ghostcell_dims[0] +
                            (k + d_num_ghosts[2])*d_ghostcell_dims[0]*d_ghostcell_dims[1];
                        
                        const int idx_density = (i + d_num_subghosts_density[0]) +
                            (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0] +
                            (k + d_num_subghosts_density[2])*d_subghostcell_dims_density[0]*d_subghostcell_dims_density[1];
                        
                        const int idx_velocity = (i + d_num_subghosts_velocity[0]) +
                            (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                            (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*d_subghostcell_dims_velocity[1];
                        
                        u[idx_velocity] = rho_u[idx]/rho[idx_density];
                        v[idx_velocity] = rho_v[idx]/rho[idx_density];
                        w[idx_velocity] = rho_w[idx]/rho[idx_density];
                    }
                }
            }
        }
    }
    else
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::computeGlobalCellDataVelocityWithDensity()\n"
            << "Cell data of 'VELOCITY' is not yet registered."
            << std::endl);
    }
}


/*
 * Compute the global cell data of internal energy with density and velocity in the registered
 * patch.
 */
void
FlowModelFourEqnConservative::computeGlobalCellDataInternalEnergyWithDensityAndVelocity()
{
    if (d_num_subghosts_internal_energy > -hier::IntVector::getOne(d_dim))
    {
        // Create the cell data of internal energy.
        d_data_internal_energy.reset(
            new pdat::CellData<double>(d_interior_box, 1, d_num_subghosts_internal_energy));
        
        // Get the cell data of the variable total energy.
        boost::shared_ptr<pdat::CellData<double> > data_total_energy =
            getGlobalCellDataTotalEnergy();
        
        if (!d_data_density)
        {
            computeGlobalCellDataDensity();
        }
        
        if (!d_data_velocity)
        {
            computeGlobalCellDataVelocityWithDensity();
        }
        
        // Get the pointers to the cell data of internal energy, total energy and density.
        double* epsilon = d_data_internal_energy->getPointer(0);
        double* E = data_total_energy->getPointer(0);
        double* rho = d_data_density->getPointer(0);
        
        if (d_dim == tbox::Dimension(1))
        {
            // Get the pointer to cell data of velocity.
            double* u = d_data_velocity->getPointer(0);
            
            // Compute the internal energy field.
            for (int i = -d_num_subghosts_internal_energy[0];
                 i < d_interior_dims[0] + d_num_subghosts_internal_energy[0];
                 i++)
            {
                // Compute the linear indices.
                const int idx = i + d_num_ghosts[0];
                const int idx_density = i + d_num_subghosts_density[0];
                const int idx_velocity = i + d_num_subghosts_velocity[0];
                const int idx_internal_energy = i + d_num_subghosts_internal_energy[0];
                
                epsilon[idx_internal_energy] = E[idx]/rho[idx_density] -
                    0.5*u[idx_velocity]*u[idx_velocity];
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            // Get the pointers to the cell data of velocity.
            double* u = d_data_velocity->getPointer(0);
            double* v = d_data_velocity->getPointer(1);
            
            // Compute the internal energy field.
            for (int j = -d_num_subghosts_internal_energy[1];
                 j < d_interior_dims[1] + d_num_subghosts_internal_energy[1];
                 j++)
            {
                for (int i = -d_num_subghosts_internal_energy[0];
                     i < d_interior_dims[0] + d_num_subghosts_internal_energy[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx = (i + d_num_ghosts[0]) +
                        (j + d_num_ghosts[1])*d_ghostcell_dims[0];
                    
                    const int idx_density = (i + d_num_subghosts_density[0]) +
                        (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0];
                    
                    const int idx_velocity = (i + d_num_subghosts_velocity[0]) +
                        (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                    
                    const int idx_internal_energy = (i + d_num_subghosts_internal_energy[0]) +
                        (j + d_num_subghosts_internal_energy[1])*d_subghostcell_dims_internal_energy[0];
                    
                    epsilon[idx_internal_energy] = E[idx]/rho[idx_density] -
                        0.5*(u[idx_velocity]*u[idx_velocity] + v[idx_velocity]*v[idx_velocity]);
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            // Get the pointers to the cell data of velocity.
            double* u = d_data_velocity->getPointer(0);
            double* v = d_data_velocity->getPointer(1);
            double* w = d_data_velocity->getPointer(2);
            
            // Compute the internal energy field.
            for (int k = -d_num_subghosts_internal_energy[2];
                 k < d_interior_dims[2] + d_num_subghosts_internal_energy[2];
                 k++)
            {
                for (int j = -d_num_subghosts_internal_energy[1];
                     j < d_interior_dims[1] + d_num_subghosts_internal_energy[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_internal_energy[0];
                         i < d_interior_dims[0] + d_num_subghosts_internal_energy[0];
                         i++)
                    {
                        // Compute the linear indices.
                        const int idx = (i + d_num_ghosts[0]) +
                            (j + d_num_ghosts[1])*d_ghostcell_dims[0] +
                            (k + d_num_ghosts[2])*d_ghostcell_dims[0]*d_ghostcell_dims[1];
                        
                        const int idx_density = (i + d_num_subghosts_density[0]) +
                            (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0] +
                            (k + d_num_subghosts_density[2])*d_subghostcell_dims_density[0]*
                                d_subghostcell_dims_density[1];
                        
                        const int idx_velocity = (i + d_num_subghosts_velocity[0]) +
                            (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                            (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                d_subghostcell_dims_velocity[1];
                        
                        const int idx_internal_energy = (i + d_num_subghosts_internal_energy[0]) +
                            (j + d_num_subghosts_internal_energy[1])*d_subghostcell_dims_internal_energy[0] +
                            (k + d_num_subghosts_internal_energy[2])*d_subghostcell_dims_internal_energy[0]*
                                d_subghostcell_dims_internal_energy[1];
                        
                        epsilon[idx_internal_energy] = E[idx]/rho[idx_density] -
                            0.5*(u[idx_velocity]*u[idx_velocity] + v[idx_velocity]*v[idx_velocity] +
                            w[idx_velocity]*w[idx_velocity]);
                    }
                }
            }
        }
    }
    else
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::"
            << "computeGlobalCellDataInternalEnergyWithDensityAndVelocity()\n"
            << "Cell data of 'INTERNAL_ENERGY' is not yet registered."
            << std::endl);
    }
}


/*
 * Compute the global cell data of pressure with density, mass fraction and internal energy in
 * the registered patch.
 */
void
FlowModelFourEqnConservative::computeGlobalCellDataPressureWithDensityMassFractionAndInternalEnergy()
{
    if (d_num_subghosts_pressure > -hier::IntVector::getOne(d_dim))
    {
        // Create the cell data of pressure.
        d_data_pressure.reset(
            new pdat::CellData<double>(d_interior_box, 1, d_num_subghosts_pressure));
        
        if (!d_data_density)
        {
            computeGlobalCellDataDensity();
        }
        
        if (!d_data_mass_fraction)
        {
            computeGlobalCellDataMassFractionWithDensity();
        }
        
        if (!d_data_internal_energy)
        {
            computeGlobalCellDataInternalEnergyWithDensityAndVelocity();
        }
        
        // Get the pointers to the cell data of pressure, density, mass fraction and internal energy.
        double* p = d_data_pressure->getPointer(0);
        double* rho = d_data_density->getPointer(0);
        std::vector<double*> Y;
        Y.reserve(d_num_species);
        for (int si = 0; si < d_num_species; si++)
        {
            Y.push_back(d_data_mass_fraction->getPointer(si));
        }
        double* epsilon = d_data_internal_energy->getPointer(0);
        
        if (d_dim == tbox::Dimension(1))
        {
            // Compute the pressure field.
            for (int i = -d_num_subghosts_pressure[0];
                 i < d_interior_dims[0] + d_num_subghosts_pressure[0];
                 i++)
            {
                // Compute the linear indices.
                const int idx_density = i + d_num_subghosts_density[0];
                const int idx_mass_fraction = i + d_num_subghosts_mass_fraction[0];
                const int idx_internal_energy = i + d_num_subghosts_internal_energy[0];
                const int idx_pressure = i + d_num_subghosts_pressure[0];
                
                std::vector<const double*> Y_ptr;
                Y_ptr.reserve(d_num_species);
                for (int si = 0; si < d_num_species; si++)
                {
                    Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                }
                
                p[idx_pressure] = d_equation_of_state_mixing_rules->
                    getPressure(
                        &rho[idx_density],
                        &epsilon[idx_internal_energy],
                        Y_ptr);
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            // Compute the pressure field.
            for (int j = -d_num_subghosts_pressure[1];
                 j < d_interior_dims[1] + d_num_subghosts_pressure[1];
                 j++)
            {
                for (int i = -d_num_subghosts_pressure[0];
                     i < d_interior_dims[0] + d_num_subghosts_pressure[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx_density = (i + d_num_subghosts_density[0]) +
                        (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0];
                    
                    const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                        (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0];
                    
                    const int idx_internal_energy = (i + d_num_subghosts_internal_energy[0]) +
                        (j + d_num_subghosts_internal_energy[1])*d_subghostcell_dims_internal_energy[0];
                    
                    const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                        (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0];
                    
                    std::vector<const double*> Y_ptr;
                    Y_ptr.reserve(d_num_species);
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                    }
                    
                    p[idx_pressure] = d_equation_of_state_mixing_rules->
                        getPressure(
                            &rho[idx_density],
                            &epsilon[idx_internal_energy],
                            Y_ptr);
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            // Compute the pressure field.
            for (int k = -d_num_subghosts_pressure[2];
                 k < d_interior_dims[2] + d_num_subghosts_pressure[2];
                 k++)
            {
                for (int j = -d_num_subghosts_pressure[1];
                     j < d_interior_dims[1] + d_num_subghosts_pressure[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_pressure[0];
                         i < d_interior_dims[0] + d_num_subghosts_pressure[0];
                         i++)
                    {
                        // Compute the linear indices.
                        const int idx_density = (i + d_num_subghosts_density[0]) +
                            (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0] +
                            (k + d_num_subghosts_density[2])*d_subghostcell_dims_density[0]*
                                d_subghostcell_dims_density[1];
                        
                        const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                            (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0] +
                            (k + d_num_subghosts_mass_fraction[2])*d_subghostcell_dims_mass_fraction[0]*
                                d_subghostcell_dims_mass_fraction[1];
                        
                        const int idx_internal_energy = (i + d_num_subghosts_internal_energy[0]) +
                            (j + d_num_subghosts_internal_energy[1])*d_subghostcell_dims_internal_energy[0] +
                            (k + d_num_subghosts_internal_energy[2])*d_subghostcell_dims_internal_energy[0]*
                                d_subghostcell_dims_internal_energy[1];
                        
                        const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                            (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0] +
                            (k + d_num_subghosts_pressure[2])*d_subghostcell_dims_pressure[0]*
                                d_subghostcell_dims_pressure[1];
                        
                        std::vector<const double*> Y_ptr;
                        Y_ptr.reserve(d_num_species);
                        for (int si = 0; si < d_num_species; si++)
                        {
                            Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                        }
                        
                        p[idx_pressure] = d_equation_of_state_mixing_rules->
                            getPressure(
                                &rho[idx_density],
                                &epsilon[idx_internal_energy],
                                Y_ptr);
                    }
                }
            }
        }
    }
    else
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::"
            << "computeGlobalCellDataPressureWithDensityMassFractionAndInternalEnergy()\n"
            << "Cell data of 'PRESSURE' is not yet registered."
            << std::endl);
    }
}


/*
 * Compute the global cell data of sound speed with density, mass fraction and pressure in the
 * registered patch.
 */
void
FlowModelFourEqnConservative::computeGlobalCellDataSoundSpeedWithDensityMassFractionAndPressure()
{
    if (d_num_subghosts_sound_speed > -hier::IntVector::getOne(d_dim))
    {
        // Create the cell data of sound speed.
        d_data_sound_speed.reset(
            new pdat::CellData<double>(d_interior_box, 1, d_num_subghosts_sound_speed));
        
        if (!d_data_density)
        {
            computeGlobalCellDataDensity();
        }
        
        if (!d_data_mass_fraction)
        {
            computeGlobalCellDataMassFractionWithDensity();
        }
        
        if (!d_data_pressure)
        {
            computeGlobalCellDataPressureWithDensityMassFractionAndInternalEnergy();
        }
        
        // Get the pointers to the cell data of sound speed, density, mass fraction and pressure.
        double* c = d_data_sound_speed->getPointer(0);
        double* rho = d_data_density->getPointer(0);
        std::vector<double*> Y;
        Y.reserve(d_num_species);
        for (int si = 0; si < d_num_species; si++)
        {
            Y.push_back(d_data_mass_fraction->getPointer(si));
        }
        double* p = d_data_pressure->getPointer(0);
        
        if (d_dim == tbox::Dimension(1))
        {
            // Compute the sound speed field.
            for (int i = -d_num_subghosts_sound_speed[0];
                 i < d_interior_dims[0] + d_num_subghosts_sound_speed[0];
                 i++)
            {
                // Compute the linear indices.
                const int idx_density = i + d_num_subghosts_density[0];
                const int idx_mass_fraction = i + d_num_subghosts_mass_fraction[0];
                const int idx_pressure = i + d_num_subghosts_pressure[0];
                const int idx_sound_speed = i + d_num_subghosts_sound_speed[0];
                
                std::vector<const double*> Y_ptr;
                Y_ptr.reserve(d_num_species);
                for (int si = 0; si < d_num_species; si++)
                {
                    Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                }
                
                c[idx_sound_speed] = d_equation_of_state_mixing_rules->
                    getSoundSpeed(
                        &rho[idx_density],
                        &p[idx_pressure],
                        Y_ptr);
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            // Compute the sound speed field.
            for (int j = -d_num_subghosts_sound_speed[1];
                 j < d_interior_dims[1] + d_num_subghosts_sound_speed[1];
                 j++)
            {
                for (int i = -d_num_subghosts_sound_speed[0];
                     i < d_interior_dims[0] + d_num_subghosts_sound_speed[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx_density = (i + d_num_subghosts_density[0]) +
                        (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0];
                    
                    const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                        (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0];
                    
                    const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                        (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0];
                    
                    const int idx_sound_speed = (i + d_num_subghosts_sound_speed[0]) +
                        (j + d_num_subghosts_sound_speed[1])*d_subghostcell_dims_sound_speed[0];
                    
                    std::vector<const double*> Y_ptr;
                    Y_ptr.reserve(d_num_species);
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                    }
                    
                    c[idx_sound_speed] = d_equation_of_state_mixing_rules->
                        getSoundSpeed(
                            &rho[idx_density],
                            &p[idx_pressure],
                            Y_ptr);
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            // Compute the sound speed field.
            for (int k = -d_num_subghosts_sound_speed[2];
                 k < d_interior_dims[2] + d_num_subghosts_sound_speed[2];
                 k++)
            {
                for (int j = -d_num_subghosts_sound_speed[1];
                     j < d_interior_dims[1] + d_num_subghosts_sound_speed[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_sound_speed[0];
                         i < d_interior_dims[0] + d_num_subghosts_sound_speed[0];
                         i++)
                    {
                        // Compute the linear indices.
                        const int idx_density = (i + d_num_subghosts_density[0]) +
                            (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0] +
                            (k + d_num_subghosts_density[2])*d_subghostcell_dims_density[0]*
                                d_subghostcell_dims_density[1];
                        
                        const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                            (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0] +
                            (k + d_num_subghosts_mass_fraction[2])*d_subghostcell_dims_mass_fraction[0]*
                                d_subghostcell_dims_mass_fraction[1];
                        
                        const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                            (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0] +
                            (k + d_num_subghosts_pressure[2])*d_subghostcell_dims_pressure[0]*
                                d_subghostcell_dims_pressure[1];
                        
                        const int idx_sound_speed = (i + d_num_subghosts_sound_speed[0]) +
                            (j + d_num_subghosts_sound_speed[1])*d_subghostcell_dims_sound_speed[0] +
                            (k + d_num_subghosts_sound_speed[2])*d_subghostcell_dims_sound_speed[0]*
                                d_subghostcell_dims_sound_speed[1];
                        
                        std::vector<const double*> Y_ptr;
                        Y_ptr.reserve(d_num_species);
                        for (int si = 0; si < d_num_species; si++)
                        {
                            Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                        }
                        
                        c[idx_sound_speed] = d_equation_of_state_mixing_rules->
                            getSoundSpeed(
                                &rho[idx_density],
                                &p[idx_pressure],
                                Y_ptr);
                    }
                }
            }
        }
    }
    else
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::"
            << "computeGlobalCellDataSoundSpeedWithDensityMassFractionAndPressure()\n"
            << "Cell data of 'SOUND_SPEED' is not yet registered."
            << std::endl);
    }
}


/*
 * Compute the global cell data of temperature with density, mass fraction and pressure in the
 * registered patch.
 */
void
FlowModelFourEqnConservative::computeGlobalCellDataTemperatureWithDensityMassFractionAndPressure()
{
    if (d_num_subghosts_temperature > -hier::IntVector::getOne(d_dim))
    {
        // Create the cell data of temperature.
        d_data_temperature.reset(
            new pdat::CellData<double>(d_interior_box, 1, d_num_subghosts_temperature));
        
        if (!d_data_density)
        {
            computeGlobalCellDataDensity();
        }
        
        if (!d_data_mass_fraction)
        {
            computeGlobalCellDataMassFractionWithDensity();
        }
        
        if (!d_data_pressure)
        {
            computeGlobalCellDataPressureWithDensityMassFractionAndInternalEnergy();
        }
        
        // Get the pointers to the cell data of temperature, density, mass fraction and pressure.
        double* T = d_data_temperature->getPointer(0);
        double* rho = d_data_density->getPointer(0);
        std::vector<double*> Y;
        Y.reserve(d_num_species);
        for (int si = 0; si < d_num_species; si++)
        {
            Y.push_back(d_data_mass_fraction->getPointer(si));
        }
        double* p = d_data_pressure->getPointer(0);
        
        if (d_dim == tbox::Dimension(1))
        {
            // Compute the sound speed field.
            for (int i = -d_num_subghosts_temperature[0];
                 i < d_interior_dims[0] + d_num_subghosts_temperature[0];
                 i++)
            {
                // Compute the linear indices.
                const int idx_density = i + d_num_subghosts_density[0];
                const int idx_mass_fraction = i + d_num_subghosts_mass_fraction[0];
                const int idx_pressure = i + d_num_subghosts_pressure[0];
                const int idx_temperature = i + d_num_subghosts_temperature[0];
                
                std::vector<const double*> Y_ptr;
                Y_ptr.reserve(d_num_species);
                for (int si = 0; si < d_num_species; si++)
                {
                    Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                }
                
                T[idx_temperature] = d_equation_of_state_mixing_rules->
                    getTemperature(
                        &rho[idx_density],
                        &p[idx_pressure],
                        Y_ptr);
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            // Compute the sound speed field.
            for (int j = -d_num_subghosts_temperature[1];
                 j < d_interior_dims[1] + d_num_subghosts_temperature[1];
                 j++)
            {
                for (int i = -d_num_subghosts_temperature[0];
                     i < d_interior_dims[0] + d_num_subghosts_temperature[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx_density = (i + d_num_subghosts_density[0]) +
                        (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0];
                    
                    const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                        (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0];
                    
                    const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                        (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0];
                    
                    const int idx_temperature = (i + d_num_subghosts_temperature[0]) +
                        (j + d_num_subghosts_temperature[1])*d_subghostcell_dims_temperature[0];
                    
                    std::vector<const double*> Y_ptr;
                    Y_ptr.reserve(d_num_species);
                    for (int si = 0; si < d_num_species; si++)
                    {
                        Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                    }
                    
                    T[idx_temperature] = d_equation_of_state_mixing_rules->
                        getTemperature(
                            &rho[idx_density],
                            &p[idx_pressure],
                            Y_ptr);
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            // Compute the sound speed field.
            for (int k = -d_num_subghosts_temperature[2];
                 k < d_interior_dims[2] + d_num_subghosts_temperature[2];
                 k++)
            {
                for (int j = -d_num_subghosts_temperature[1];
                     j < d_interior_dims[1] + d_num_subghosts_temperature[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_temperature[0];
                         i < d_interior_dims[0] + d_num_subghosts_temperature[0];
                         i++)
                    {
                        // Compute the linear indices.
                        const int idx_density = (i + d_num_subghosts_density[0]) +
                            (j + d_num_subghosts_density[1])*d_subghostcell_dims_density[0] +
                            (k + d_num_subghosts_density[2])*d_subghostcell_dims_density[0]*
                                d_subghostcell_dims_density[1];
                        
                        const int idx_mass_fraction = (i + d_num_subghosts_mass_fraction[0]) +
                            (j + d_num_subghosts_mass_fraction[1])*d_subghostcell_dims_mass_fraction[0] +
                            (k + d_num_subghosts_mass_fraction[2])*d_subghostcell_dims_mass_fraction[0]*
                                d_subghostcell_dims_mass_fraction[1];
                        
                        const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                            (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0] +
                            (k + d_num_subghosts_pressure[2])*d_subghostcell_dims_pressure[0]*
                                d_subghostcell_dims_pressure[1];
                        
                        const int idx_temperature = (i + d_num_subghosts_temperature[0]) +
                            (j + d_num_subghosts_temperature[1])*d_subghostcell_dims_temperature[0] +
                            (k + d_num_subghosts_temperature[2])*d_subghostcell_dims_temperature[0]*
                                d_subghostcell_dims_temperature[1];
                        
                        std::vector<const double*> Y_ptr;
                        Y_ptr.reserve(d_num_species);
                        for (int si = 0; si < d_num_species; si++)
                        {
                            Y_ptr.push_back(&Y[si][idx_mass_fraction]);
                        }
                        
                        T[idx_temperature] = d_equation_of_state_mixing_rules->
                            getTemperature(
                                &rho[idx_density],
                                &p[idx_pressure],
                                Y_ptr);
                    }
                }
            }
        }
    }
    else
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::"
            << "computeGlobalCellDataTemperatureWithDensityMassFractionAndPressure()\n"
            << "Cell data of 'TEMPERATURE' is not yet registered."
            << std::endl);
    }
}


/*
 * Compute the global cell data of dilatation with density and velocity in the registered patch.
 */
void
FlowModelFourEqnConservative::computeGlobalCellDataDilatationWithDensityAndVelocity()
{
    if (d_num_subghosts_dilatation > -hier::IntVector::getOne(d_dim))
    {
        // Get the grid spacing.
        const boost::shared_ptr<geom::CartesianPatchGeometry> patch_geom(
            BOOST_CAST<geom::CartesianPatchGeometry, hier::PatchGeometry>(
                d_patch->getPatchGeometry()));
        
        const double* const dx = patch_geom->getDx();
        
        // Create the cell data of dilatation.
        d_data_dilatation.reset(
            new pdat::CellData<double>(d_interior_box, 1, d_num_subghosts_dilatation));
        
        if (!d_data_density)
        {
            computeGlobalCellDataDensity();
        }
        
        if (!d_data_velocity)
        {
            computeGlobalCellDataVelocityWithDensity();
        }
        
        // Get the pointer to the cell data of dilatation.
        double* theta = d_data_dilatation->getPointer(0);
        
        if (d_dim == tbox::Dimension(1))
        {
            // Get the pointer to cell data of velocity.
            double* u = d_data_velocity->getPointer(0);
            
            // Compute the dilatation field.
            if (d_num_subghosts_dilatation < d_num_subghosts_velocity)
            {
                for (int i = -d_num_subghosts_dilatation[0];
                     i < d_interior_dims[0] + d_num_subghosts_dilatation[0];
                     i++)
                {
                    // Compute indices of current and neighboring cells.
                    const int idx_x_L = i - 1 + d_num_subghosts_velocity[0];
                    const int idx_x_R = i + 1 + d_num_subghosts_velocity[0];
                    const int idx_dilatation = i + d_num_subghosts_dilatation[0];
                    
                    theta[idx_dilatation] = (u[idx_x_R] - u[idx_x_L])/(2*dx[0]);
                }
            }
            else
            {
                for (int i = -d_num_subghosts_dilatation[0];
                     i < d_interior_dims[0] + d_num_subghosts_dilatation[0];
                     i++)
                {
                    // Compute indices of current and neighboring cells.
                    const int idx = i + d_num_subghosts_velocity[0];
                    const int idx_x_L = i - 1 + d_num_subghosts_velocity[0];
                    const int idx_x_R = i + 1 + d_num_subghosts_velocity[0];
                    const int idx_dilatation = i + d_num_subghosts_dilatation[0];
                    
                    if (i == -d_num_subghosts_velocity[0])
                    {
                        theta[idx_dilatation] = (u[idx_x_R] - u[idx])/(dx[0]);
                    }
                    else if (i == d_interior_dims[0] + d_num_subghosts_velocity[0] - 1)
                    {
                        theta[idx_dilatation] = (u[idx] - u[idx_x_L])/(dx[0]);
                    }
                    else
                    {
                        theta[idx_dilatation] = (u[idx_x_R] - u[idx_x_L])/(2*dx[0]);
                    }
                }
            }
        }
        else if (d_dim == tbox::Dimension(2))
        {
            // Get the pointers to the cell data of velocity.
            double* u = d_data_velocity->getPointer(0);
            double* v = d_data_velocity->getPointer(1);
            
            // Compute the dilatation field.
            if (d_num_subghosts_dilatation < d_num_subghosts_velocity)
            {
                for (int j = -d_num_subghosts_dilatation[1];
                     j < d_interior_dims[1] + d_num_subghosts_dilatation[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_dilatation[0];
                         i < d_interior_dims[0] + d_num_subghosts_dilatation[0];
                         i++)
                    {
                        // Compute indices of current and neighboring cells.
                        const int idx_x_L = (i - 1 + d_num_subghosts_velocity[0]) +
                            (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_x_R = (i + 1 + d_num_subghosts_velocity[0]) +
                            (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_y_B = (i + d_num_subghosts_velocity[0]) +
                            (j - 1 + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_y_T = (i + d_num_subghosts_velocity[0]) +
                            (j + 1 + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_dilatation = (i + d_num_subghosts_dilatation[0]) +
                            (j + d_num_subghosts_dilatation[1])*d_subghostcell_dims_dilatation[0];
                        
                        double dudx = (u[idx_x_R] - u[idx_x_L])/(2*dx[0]);
                        double dvdy = (v[idx_y_T] - v[idx_y_B])/(2*dx[1]);
                        
                        theta[idx_dilatation] = dudx + dvdy;
                    }
                }
            }
            else
            {
                for (int j = -d_num_subghosts_dilatation[1];
                     j < d_interior_dims[1] + d_num_subghosts_dilatation[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_dilatation[0];
                         i < d_interior_dims[0] + d_num_subghosts_dilatation[0];
                         i++)
                    {
                        // Compute indices of current and neighboring cells.
                        const int idx = (i + d_num_subghosts_velocity[0]) +
                            (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_x_L = (i - 1 + d_num_subghosts_velocity[0]) +
                            (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_x_R = (i + 1 + d_num_subghosts_velocity[0]) +
                            (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_y_B = (i + d_num_subghosts_velocity[0]) +
                            (j - 1 + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_y_T = (i + d_num_subghosts_velocity[0]) +
                            (j + 1 + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_dilatation = (i + d_num_subghosts_dilatation[0]) +
                            (j + d_num_subghosts_dilatation[1])*d_subghostcell_dims_dilatation[0];
                        
                        double dudx, dvdy;
                        
                        if (i == -d_num_subghosts_velocity[0])
                        {
                            dudx = (u[idx_x_R] - u[idx])/(dx[0]);
                        }
                        else if (i == d_interior_dims[0] + d_num_subghosts_velocity[0] - 1)
                        {
                            dudx = (u[idx] - u[idx_x_L])/(dx[0]);
                        }
                        else
                        {
                            dudx = (u[idx_x_R] - u[idx_x_L])/(2*dx[0]);
                        }
                        
                        if (j == -d_num_subghosts_velocity[1])
                        {
                            dvdy = (v[idx_y_T] - v[idx])/(dx[1]);
                        }
                        else if (j == d_interior_dims[1] + d_num_subghosts_velocity[1] - 1)
                        {
                            dvdy = (v[idx] - v[idx_y_B])/(dx[1]);
                        }
                        else
                        {
                            dvdy = (v[idx_y_T] - v[idx_y_B])/(2*dx[1]);
                        }
                        
                        theta[idx_dilatation] = dudx + dvdy;
                    }
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            // Get the pointers to the cell data of velocity.
            double* u = d_data_velocity->getPointer(0);
            double* v = d_data_velocity->getPointer(1);
            double* w = d_data_velocity->getPointer(2);
            
            // Compute the dilatation field.
            if (d_num_subghosts_dilatation < d_num_subghosts_velocity)
            {
                for (int k = -d_num_subghosts_dilatation[2];
                     k < d_interior_dims[2] + d_num_subghosts_dilatation[2];
                     k++)
                {
                    for (int j = -d_num_subghosts_dilatation[1];
                         j < d_interior_dims[1] + d_num_subghosts_dilatation[1];
                         j++)
                    {
                        for (int i = -d_num_subghosts_dilatation[0];
                             i < d_interior_dims[0] + d_num_subghosts_dilatation[0];
                             i++)
                        {
                            // Compute indices of current and neighboring cells.
                            const int idx_x_L = (i - 1 + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_x_R = (i + 1 + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_y_B = (i + d_num_subghosts_velocity[0]) +
                                (j - 1 + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_y_T = (i + d_num_subghosts_velocity[0]) +
                                (j + 1 + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_z_B = (i + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k - 1 + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_z_F = (i + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + 1 + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_dilatation = (i + d_num_subghosts_dilatation[0]) +
                                (j + d_num_subghosts_dilatation[1])*d_subghostcell_dims_dilatation[0] +
                                (k + d_num_subghosts_dilatation[2])*d_subghostcell_dims_dilatation[0]*
                                    d_subghostcell_dims_dilatation[1];
                            
                            double dudx = (u[idx_x_R] - u[idx_x_L])/(2*dx[0]);
                            double dvdy = (v[idx_y_T] - v[idx_y_B])/(2*dx[1]);
                            double dwdz = (w[idx_z_F] - w[idx_z_B])/(2*dx[2]);
                            
                            theta[idx_dilatation] = dudx + dvdy + dwdz;
                        }
                    }
                }
            }
            else
            {
                for (int k = -d_num_subghosts_dilatation[2];
                     k < d_interior_dims[2] + d_num_subghosts_dilatation[2];
                     k++)
                {
                    for (int j = -d_num_subghosts_dilatation[1];
                         j < d_interior_dims[1] + d_num_subghosts_dilatation[1];
                         j++)
                    {
                        for (int i = -d_num_subghosts_dilatation[0];
                             i < d_interior_dims[0] + d_num_subghosts_dilatation[0];
                             i++)
                        {
                            // Compute indices of current and neighboring cells.
                            const int idx = (i + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_x_L = (i - 1 + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_x_R = (i + 1 + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_y_B = (i + d_num_subghosts_velocity[0]) +
                                (j - 1 + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_y_T = (i + d_num_subghosts_velocity[0]) +
                                (j + 1 + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_z_B = (i + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k - 1 + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_z_F = (i + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + 1 + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_dilatation = (i + d_num_subghosts_dilatation[0]) +
                                (j + d_num_subghosts_dilatation[1])*d_subghostcell_dims_dilatation[0] +
                                (k + d_num_subghosts_dilatation[2])*d_subghostcell_dims_dilatation[0]*
                                    d_subghostcell_dims_dilatation[1];
                            
                            double dudx, dvdy, dwdz;
                            
                            if (i == -d_num_subghosts_velocity[0])
                            {
                                dudx = (u[idx_x_R] - u[idx])/(dx[0]);
                            }
                            else if (i == d_interior_dims[0] + d_num_subghosts_velocity[0] - 1)
                            {
                                dudx = (u[idx] - u[idx_x_L])/(dx[0]);
                            }
                            else
                            {
                                dudx = (u[idx_x_R] - u[idx_x_L])/(2*dx[0]);
                            }
                            
                            if (j == -d_num_subghosts_velocity[1])
                            {
                                dvdy = (v[idx_y_T] - v[idx])/(dx[1]);
                            }
                            else if (j == d_interior_dims[1] + d_num_subghosts_velocity[1] - 1)
                            {
                                dvdy = (v[idx] - v[idx_y_B])/(dx[1]);
                            }
                            else
                            {
                                dvdy = (v[idx_y_T] - v[idx_y_B])/(2*dx[1]);
                            }
                            
                            if (k == -d_num_subghosts_velocity[2])
                            {
                                dwdz = (w[idx_z_F] - w[idx])/(dx[2]);
                            }
                            else if (k == d_interior_dims[2] + d_num_subghosts_velocity[2] - 1)
                            {
                                dwdz = (w[idx] - w[idx_z_B])/(dx[2]);
                            }
                            else
                            {
                                dwdz = (w[idx_z_F] - w[idx_z_B])/(2*dx[2]);
                            }
                            
                            theta[idx_dilatation] = dudx + dvdy + dwdz;
                        }
                    }
                }
            }
        }
    }
    else
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::computeGlobalCellDataDilatationWithDensityAndVelocity()\n"
            << "Cell data of 'DILATATION' is not yet registered."
            << std::endl);
    }
}


/*
 * Compute the global cell data of vorticity with density and velocity in the registered patch.
 */
void
FlowModelFourEqnConservative::computeGlobalCellDataVorticityWithDensityAndVelocity()
{
    if (d_num_subghosts_vorticity > -hier::IntVector::getOne(d_dim))
    {
        // Get the grid spacing.
        const boost::shared_ptr<geom::CartesianPatchGeometry> patch_geom(
            BOOST_CAST<geom::CartesianPatchGeometry, hier::PatchGeometry>(
                d_patch->getPatchGeometry()));
        
        const double* const dx = patch_geom->getDx();
        
        if (!d_data_density)
        {
            computeGlobalCellDataDensity();
        }
        
        if (!d_data_velocity)
        {
            computeGlobalCellDataVelocityWithDensity();
        }
        
        if (d_dim == tbox::Dimension(1))
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::computeGlobalCellDataVorticityWithDensityAndVelocity()\n"
                << "Vorticity cannot be found for one-dimensional flow."
                << std::endl);
        }
        else if (d_dim == tbox::Dimension(2))
        {
            d_data_vorticity.reset(
                new pdat::CellData<double>(d_interior_box, 1, d_num_subghosts_vorticity));
            
            // Get the pointer to the cell data of vorticity.
            double* omega = d_data_vorticity->getPointer(0);
            
            // Get the pointers to the cell data of velocity.
            double* u = d_data_velocity->getPointer(0);
            double* v = d_data_velocity->getPointer(1);
            
            // Compute the vorticity field.
            if (d_num_subghosts_vorticity < d_num_subghosts_velocity)
            {
                for (int j = -d_num_subghosts_vorticity[1];
                     j < d_interior_dims[1] + d_num_subghosts_vorticity[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_vorticity[0];
                         i < d_interior_dims[0] + d_num_subghosts_vorticity[0];
                         i++)
                    {
                        // Compute indices of current and neighboring cells.
                        const int idx_x_L = (i - 1 + d_num_subghosts_velocity[0]) +
                            (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_x_R = (i + 1 + d_num_subghosts_velocity[0]) +
                            (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_y_B = (i + d_num_subghosts_velocity[0]) +
                            (j - 1 + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_y_T = (i + d_num_subghosts_velocity[0]) +
                            (j + 1 + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_vorticity = (i + d_num_subghosts_vorticity[0]) +
                            (j + d_num_subghosts_vorticity[1])*d_subghostcell_dims_vorticity[0];
                        
                        double dvdx = (v[idx_x_R] - v[idx_x_L])/(2*dx[0]);
                        double dudy = (u[idx_y_T] - u[idx_y_B])/(2*dx[1]);
                        
                        omega[idx_vorticity] = dvdx - dudy;
                    }
                }
            }
            else
            {
                for (int j = -d_num_subghosts_vorticity[1];
                     j < d_interior_dims[1] + d_num_subghosts_vorticity[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_vorticity[0];
                         i < d_interior_dims[0] + d_num_subghosts_vorticity[0];
                         i++)
                    {
                        // Compute indices of current and neighboring cells.
                        const int idx = (i + d_num_subghosts_velocity[0]) +
                            (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_x_L = (i - 1 + d_num_subghosts_velocity[0]) +
                            (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_x_R = (i + 1 + d_num_subghosts_velocity[0]) +
                            (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_y_B = (i + d_num_subghosts_velocity[0]) +
                            (j - 1 + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_y_T = (i + d_num_subghosts_velocity[0]) +
                            (j + 1 + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_vorticity = (i + d_num_subghosts_vorticity[0]) +
                            (j + d_num_subghosts_vorticity[1])*d_subghostcell_dims_vorticity[0];
                        
                        double dudy, dvdx;
                        
                        if (i == -d_num_subghosts_velocity[0])
                        {
                            dvdx = (v[idx_x_R] - v[idx])/(dx[0]);
                        }
                        else if (i == d_interior_dims[0] + d_num_subghosts_velocity[0] - 1)
                        {
                            dvdx = (v[idx] - v[idx_x_L])/(dx[0]);
                        }
                        else
                        {
                            dvdx = (v[idx_x_R] - v[idx_x_L])/(2*dx[0]);
                        }
                        
                        if (j == -d_num_subghosts_velocity[1])
                        {
                            dudy = (u[idx_y_T] - u[idx])/(dx[1]);
                        }
                        else if (j == d_interior_dims[1] + d_num_subghosts_velocity[1] - 1)
                        {
                            dudy = (u[idx] - u[idx_y_B])/(dx[1]);
                        }
                        else
                        {
                            dudy = (u[idx_y_T] - u[idx_y_B])/(2*dx[1]);
                        }
                        
                        omega[idx_vorticity] = dvdx - dudy;
                    }
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            d_data_vorticity.reset(
                new pdat::CellData<double>(d_interior_box, 3, d_num_subghosts_vorticity));
            
            // Get the pointers to the cell data of vorticity.
            double* omega_x = d_data_vorticity->getPointer(0);
            double* omega_y = d_data_vorticity->getPointer(1);
            double* omega_z = d_data_vorticity->getPointer(2);
            
            // Get the pointers to the cell data of velocity.
            double* u = d_data_velocity->getPointer(0);
            double* v = d_data_velocity->getPointer(1);
            double* w = d_data_velocity->getPointer(2);
            
            // Compute the vorticity field.
            if (d_num_subghosts_vorticity < d_num_subghosts_velocity)
            {
                for (int k = -d_num_subghosts_vorticity[2];
                     k < d_interior_dims[2] + d_num_subghosts_vorticity[2];
                     k++)
                {
                    for (int j = -d_num_subghosts_vorticity[1];
                         j < d_interior_dims[1] + d_num_subghosts_vorticity[1];
                         j++)
                    {
                        for (int i = -d_num_subghosts_vorticity[0];
                             i < d_interior_dims[0] + d_num_subghosts_vorticity[0];
                             i++)
                        {
                            // Compute indices of current and neighboring cells.
                            const int idx_x_L = (i - 1 + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_x_R = (i + 1 + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_y_B = (i + d_num_subghosts_velocity[0]) +
                                (j - 1 + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_y_T = (i + d_num_subghosts_velocity[0]) +
                                (j + 1 + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_z_B = (i + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k - 1 + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_z_F = (i + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + 1 + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_vorticity = (i + d_num_subghosts_vorticity[0]) +
                                (j + d_num_subghosts_vorticity[1])*d_subghostcell_dims_vorticity[0] +
                                (k + d_num_subghosts_vorticity[2])*d_subghostcell_dims_vorticity[0]*
                                    d_subghostcell_dims_vorticity[1];
                            
                            double dvdx = (v[idx_x_R] - v[idx_x_L])/(2*dx[0]);
                            double dwdx = (w[idx_x_R] - w[idx_x_L])/(2*dx[0]);
                            double dudy = (u[idx_y_T] - u[idx_y_B])/(2*dx[1]);
                            double dwdy = (w[idx_y_T] - w[idx_y_B])/(2*dx[1]);
                            double dudz = (u[idx_z_F] - u[idx_z_B])/(2*dx[2]);
                            double dvdz = (v[idx_z_F] - v[idx_z_B])/(2*dx[2]);
                            
                            omega_x[idx_vorticity] = dwdy - dvdz;
                            omega_y[idx_vorticity] = dudz - dwdx;
                            omega_z[idx_vorticity] = dvdx - dudy;
                        }
                    }
                }
            }
            else
            {
                for (int k = -d_num_subghosts_vorticity[2]; k < d_interior_dims[2] + d_num_subghosts_vorticity[2]; k++)
                {
                    for (int j = -d_num_subghosts_vorticity[1]; j < d_interior_dims[1] + d_num_subghosts_vorticity[1]; j++)
                    {
                        for (int i = -d_num_subghosts_vorticity[0]; i < d_interior_dims[0] + d_num_subghosts_vorticity[0]; i++)
                        {
                            // Compute indices of current and neighboring cells.
                            const int idx = (i + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_x_L = (i - 1 + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_x_R = (i + 1 + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_y_B = (i + d_num_subghosts_velocity[0]) +
                                (j - 1 + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_y_T = (i + d_num_subghosts_velocity[0]) +
                                (j + 1 + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_z_B = (i + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k - 1 + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_z_F = (i + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + 1 + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_vorticity = (i + d_num_subghosts_vorticity[0]) +
                                (j + d_num_subghosts_vorticity[1])*d_subghostcell_dims_vorticity[0] +
                                (k + d_num_subghosts_vorticity[2])*d_subghostcell_dims_vorticity[0]*
                                    d_subghostcell_dims_vorticity[1];
                            
                            double dudy, dudz, dvdx, dvdz, dwdx, dwdy;
                            
                            if (i == -d_num_subghosts_velocity[0])
                            {
                                dvdx = (v[idx_x_R] - v[idx])/(dx[0]);
                                dwdx = (w[idx_x_R] - w[idx])/(dx[0]);
                            }
                            else if (i == d_interior_dims[0] + d_num_subghosts_velocity[0] - 1)
                            {
                                dvdx = (v[idx] - v[idx_x_L])/(dx[0]);
                                dwdx = (w[idx] - w[idx_x_L])/(dx[0]);
                            }
                            else
                            {
                                dvdx = (v[idx_x_R] - v[idx_x_L])/(2*dx[0]);
                                dwdx = (w[idx_x_R] - w[idx_x_L])/(2*dx[0]);
                            }
                            
                            if (j == -d_num_subghosts_velocity[1])
                            {
                                dudy = (u[idx_y_T] - u[idx])/(dx[1]);
                                dwdy = (w[idx_y_T] - w[idx])/(dx[1]);
                            }
                            else if (j == d_interior_dims[1] + d_num_subghosts_velocity[1] - 1)
                            {
                                dudy = (u[idx] - u[idx_y_B])/(dx[1]);
                                dwdy = (w[idx] - w[idx_y_B])/(dx[1]);
                            }
                            else
                            {
                                dudy = (u[idx_y_T] - u[idx_y_B])/(2*dx[1]);
                                dwdy = (w[idx_y_T] - w[idx_y_B])/(2*dx[1]);
                            }
                            
                            if (k == -d_num_subghosts_velocity[2])
                            {
                                dudz = (u[idx_z_F] - u[idx])/(dx[2]);
                                dvdz = (v[idx_z_F] - v[idx])/(dx[2]);
                            }
                            else if (k == d_interior_dims[2] + d_num_subghosts_velocity[2] - 1)
                            {
                                dudz = (u[idx] - u[idx_z_B])/(dx[2]);
                                dvdz = (v[idx] - v[idx_z_B])/(dx[2]);
                            }
                            else
                            {
                                dudz = (u[idx_z_F] - u[idx_z_B])/(2*dx[2]);
                                dvdz = (v[idx_z_F] - v[idx_z_B])/(2*dx[2]);
                            }
                            
                            omega_x[idx_vorticity] = dwdy - dvdz;
                            omega_y[idx_vorticity] = dudz - dwdx;
                            omega_z[idx_vorticity] = dvdx - dudy;
                        }
                    }
                }
            }
        }
    }
    else
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::computeGlobalCellDataVorticityWithDensityAndVelocity()\n"
            << "Cell data of 'VORTICITY' is not yet registered."
            << std::endl);
    }
}


/*
 * Compute the global cell data of enstrophy with vorticity in the registered patch.
 */
void
FlowModelFourEqnConservative::computeGlobalCellDataEnstrophyWithVorticity()
{
    if (d_num_subghosts_enstrophy > -hier::IntVector::getOne(d_dim))
    {
        // Create the cell data of enstrophy.
        d_data_enstrophy.reset(
            new pdat::CellData<double>(d_interior_box, 1, d_num_subghosts_enstrophy));
        
        // Get the cell data of the vorticity.
        if (!d_data_vorticity) // If the pointer is null.
        {
            computeGlobalCellDataVorticityWithDensityAndVelocity();
        }
        
        // Get the pointer to the cell data of enstrophy.
        double* Omega = d_data_enstrophy->getPointer(0);
        
        if (d_dim == tbox::Dimension(1))
        {
            TBOX_ERROR(d_object_name
                << ": "
                << "Enstrophy cannot be found for one-dimensional flow."
                << std::endl);
        }
        else if (d_dim == tbox::Dimension(2))
        {
            // Get the pointer to the cell data of vorticity.
            double* omega = d_data_vorticity->getPointer(0);
            
            // Compute the enstrophy field.
            for (int j = -d_num_subghosts_enstrophy[1];
                 j < d_interior_dims[1] + d_num_subghosts_enstrophy[1];
                 j++)
            {
                for (int i = -d_num_subghosts_enstrophy[0];
                     i < d_interior_dims[0] + d_num_subghosts_enstrophy[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx_vorticity = (i + d_num_subghosts_vorticity[0]) +
                        (j + d_num_subghosts_vorticity[1])*d_subghostcell_dims_vorticity[0];
                    
                    const int idx_enstrophy = (i + d_num_subghosts_enstrophy[0]) +
                        (j + d_num_subghosts_enstrophy[1])*d_subghostcell_dims_enstrophy[0];
                    
                    Omega[idx_enstrophy] = 0.5*omega[idx_vorticity]*omega[idx_vorticity];
                }
            }
        }
        else if (d_dim == tbox::Dimension(3))
        {
            // Get the pointers to the cell data of vorticity.
            double* omega_x = d_data_vorticity->getPointer(0);
            double* omega_y = d_data_vorticity->getPointer(1);
            double* omega_z = d_data_vorticity->getPointer(2);
            
            // Compute the enstrophy field.
            for (int k = -d_num_subghosts_enstrophy[2];
                 k < d_interior_dims[2] + d_num_subghosts_enstrophy[2];
                 k++)
            {
                for (int j = -d_num_subghosts_enstrophy[1];
                     j < d_interior_dims[1] + d_num_subghosts_enstrophy[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_enstrophy[0];
                         i < d_interior_dims[0] + d_num_subghosts_enstrophy[0];
                         i++)
                    {
                        // Compute the linear indices.
                        const int idx_vorticity = (i + d_num_subghosts_vorticity[0]) +
                                (j + d_num_subghosts_vorticity[1])*d_subghostcell_dims_vorticity[0] +
                                (k + d_num_subghosts_vorticity[2])*d_subghostcell_dims_vorticity[0]*
                                    d_subghostcell_dims_vorticity[1];
                        
                        const int idx_enstrophy = (i + d_num_subghosts_enstrophy[0]) +
                                (j + d_num_subghosts_enstrophy[1])*d_subghostcell_dims_enstrophy[0] +
                                (k + d_num_subghosts_enstrophy[2])*d_subghostcell_dims_enstrophy[0]*
                                    d_subghostcell_dims_enstrophy[1];
                        
                        Omega[idx_enstrophy] = 0.5*(omega_x[idx_vorticity]*omega_x[idx_vorticity] +
                            omega_y[idx_vorticity]*omega_y[idx_vorticity] +
                            omega_z[idx_vorticity]*omega_z[idx_vorticity]);
                    }
                }
            }
        }
    }
    else
    {
        TBOX_ERROR(d_object_name
            << ": FlowModelFourEqnConservative::computeGlobalCellDataEnstrophyWithDensityVelocityAndVorticity()\n"
            << "Cell data of 'ENSTROPHY' is not yet registered."
            << std::endl);
    }
}


/*
 * Compute the global cell data of convective flux with velocity and pressure in the registered
 * patch.
 */
void
FlowModelFourEqnConservative::computeGlobalCellDataConvectiveFluxWithVelocityAndPressure(
    DIRECTION::TYPE direction)
{
    if (direction == DIRECTION::X_DIRECTION)
    {
        if (d_num_subghosts_convective_flux_x > -hier::IntVector::getOne(d_dim))
        {
            // Create the cell data of convective flux in the x-direction.
            d_data_convective_flux_x.reset(
                new pdat::CellData<double>(d_interior_box, d_num_eqn, d_num_subghosts_convective_flux_x));
            
            // Get the pointers to the components of the convective flux in the x-direction.
            std::vector<double*> F_x;
            F_x.reserve(d_num_eqn);
            for (int ei = 0; ei < d_num_eqn; ei++)
            {
                F_x.push_back(d_data_convective_flux_x->getPointer(ei));
            }
            
            boost::shared_ptr<pdat::CellData<double> > data_partial_density =
                getGlobalCellDataPartialDensity();
            
            boost::shared_ptr<pdat::CellData<double> > data_momentum =
                getGlobalCellDataMomentum();
            
            boost::shared_ptr<pdat::CellData<double> > data_total_energy =
                getGlobalCellDataTotalEnergy();
            
            if (!d_data_velocity)
            {
                computeGlobalCellDataVelocityWithDensity();
            }
            
            if (!d_data_pressure)
            {
                computeGlobalCellDataPressureWithDensityMassFractionAndInternalEnergy();
            }
            
            // Get the pointers to the cell data of partial density, total energy and pressure.
            std::vector<double*> rho_Y;
            rho_Y.reserve(d_num_species);
            for (int si = 0; si < d_num_species; si++)
            {
                rho_Y.push_back(data_partial_density->getPointer(si));
            }
            double* E = data_total_energy->getPointer(0);
            double* p = d_data_pressure->getPointer(0);
            
            if (d_dim == tbox::Dimension(1))
            {
                // Get the pointer to the cell data of momentum.
                double* rho_u = data_momentum->getPointer(0);
                
                // Get the pointer to the cell data of velocity.
                double* u = d_data_velocity->getPointer(0);
                
                // Compute the convective flux in the x-direction.
                for (int i = -d_num_subghosts_convective_flux_x[0];
                     i < d_interior_dims[0] + d_num_subghosts_convective_flux_x[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx = i + d_num_ghosts[0];
                    const int idx_pressure = i + d_num_subghosts_pressure[0];
                    const int idx_velocity = i + d_num_subghosts_velocity[0];
                    const int idx_convective_flux_x = i + d_num_subghosts_convective_flux_x[0];
                    
                    for (int si = 0; si < d_num_species; si++)
                    {
                        F_x[si][idx_convective_flux_x] = u[idx_velocity]*rho_Y[si][idx];
                    }
                    F_x[d_num_species][idx_convective_flux_x] = u[idx_velocity]*rho_u[idx] + p[idx_pressure];
                    F_x[d_num_species + 1][idx_convective_flux_x] = u[idx_velocity]*(E[idx] + p[idx_pressure]);
                }
            }
            else if (d_dim == tbox::Dimension(2))
            {
                // Get the pointers to the cell data of momentum.
                double* rho_u = data_momentum->getPointer(0);
                double* rho_v = data_momentum->getPointer(1);
                
                // Get the pointer to the cell data of velocity.
                double* u = d_data_velocity->getPointer(0);
                
                // Compute the convective flux in the x-direction.
                for (int j = -d_num_subghosts_convective_flux_x[1];
                     j < d_interior_dims[1] + d_num_subghosts_convective_flux_x[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_convective_flux_x[0];
                         i < d_interior_dims[0] + d_num_subghosts_convective_flux_x[0];
                         i++)
                    {
                        const int idx = (i + d_num_ghosts[0]) +
                        (j + d_num_ghosts[1])*d_ghostcell_dims[0];
                        
                        const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                            (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0];
                        
                        const int idx_velocity = (i + d_num_subghosts_velocity[0]) +
                            (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_convective_flux_x = (i + d_num_subghosts_convective_flux_x[0]) +
                            (j + d_num_subghosts_convective_flux_x[1])*d_subghostcell_dims_convective_flux_x[0];
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            F_x[si][idx_convective_flux_x] = u[idx_velocity]*rho_Y[si][idx];
                        }
                        F_x[d_num_species][idx_convective_flux_x] = u[idx_velocity]*rho_u[idx] + p[idx_pressure];
                        F_x[d_num_species + 1][idx_convective_flux_x] = u[idx_velocity]*rho_v[idx];
                        F_x[d_num_species + 2][idx_convective_flux_x] = u[idx_velocity]*(E[idx] + p[idx_pressure]);
                    }
                }
            }
            else if (d_dim == tbox::Dimension(3))
            {
                // Get the pointers to the cell data of momentum.
                double* rho_u = data_momentum->getPointer(0);
                double* rho_v = data_momentum->getPointer(1);
                double* rho_w = data_momentum->getPointer(2);
                
                // Get the pointer to the cell data of velocity.
                double* u = d_data_velocity->getPointer(0);
                
                // Compute the convective flux in the x-direction.
                for (int k = -d_num_subghosts_convective_flux_x[2];
                     k < d_interior_dims[2] + d_num_subghosts_convective_flux_x[2];
                     k++)
                {
                    for (int j = -d_num_subghosts_convective_flux_x[1];
                         j < d_interior_dims[1] + d_num_subghosts_convective_flux_x[1];
                         j++)
                    {
                        for (int i = -d_num_subghosts_convective_flux_x[0];
                             i < d_interior_dims[0] + d_num_subghosts_convective_flux_x[0];
                             i++)
                        {
                            // Compute the linear indices.
                            const int idx = (i + d_num_ghosts[0]) +
                                (j + d_num_ghosts[1])*d_ghostcell_dims[0] +
                                (k + d_num_ghosts[2])*d_ghostcell_dims[0]*d_ghostcell_dims[1];
                            
                            const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                                (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0] +
                                (k + d_num_subghosts_pressure[2])*d_subghostcell_dims_pressure[0]*
                                    d_subghostcell_dims_pressure[1];
                            
                            const int idx_velocity = (i + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_convective_flux_x = (i + d_num_subghosts_convective_flux_x[0]) +
                                (j + d_num_subghosts_convective_flux_x[1])*d_subghostcell_dims_convective_flux_x[0] +
                                (k + d_num_subghosts_convective_flux_x[2])*d_subghostcell_dims_convective_flux_x[0]*
                                    d_subghostcell_dims_convective_flux_x[1];
                            
                            for (int si = 0; si < d_num_species; si++)
                            {
                                F_x[si][idx_convective_flux_x] = u[idx_velocity]*rho_Y[si][idx];
                            }
                            F_x[d_num_species][idx_convective_flux_x] = u[idx_velocity]*rho_u[idx] + p[idx_pressure];
                            F_x[d_num_species + 1][idx_convective_flux_x] = u[idx_velocity]*rho_v[idx];
                            F_x[d_num_species + 2][idx_convective_flux_x] = u[idx_velocity]*rho_w[idx];
                            F_x[d_num_species + 3][idx_convective_flux_x] = u[idx_velocity]*(E[idx] + p[idx_pressure]);
                        }
                    }
                }
            }
        }
        else
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::"
                << "computeGlobalCellDataConvectiveFluxWithVelocityAndPressure()\n"
                << "Cell data of 'CONVECTIVE_FLUX_X' is not yet registered."
                << std::endl);
        }
    }
    else if (direction == DIRECTION::Y_DIRECTION)
    {
        if (d_num_subghosts_convective_flux_y > -hier::IntVector::getOne(d_dim))
        {
            // Create the cell data of convective flux in the y-direction.
            d_data_convective_flux_y.reset(
                new pdat::CellData<double>(d_interior_box, d_num_eqn, d_num_subghosts_convective_flux_y));
            
            // Get the pointers to the components of the convective flux in the y-direction.
            std::vector<double*> F_y;
            F_y.reserve(d_num_eqn);
            for (int ei = 0; ei < d_num_eqn; ei++)
            {
                F_y.push_back(d_data_convective_flux_y->getPointer(ei));
            }
            
            boost::shared_ptr<pdat::CellData<double> > data_partial_density =
                getGlobalCellDataPartialDensity();
            
            boost::shared_ptr<pdat::CellData<double> > data_momentum =
                getGlobalCellDataMomentum();
            
            boost::shared_ptr<pdat::CellData<double> > data_total_energy =
                getGlobalCellDataTotalEnergy();
            
            if (!d_data_pressure)
            {
                computeGlobalCellDataPressureWithDensityMassFractionAndInternalEnergy();
            }
            
            if (!d_data_velocity)
            {
                computeGlobalCellDataVelocityWithDensity();
            }
            
            // Get the pointers to the cell data of partial density, total energy, volume fraction
            // and pressure.
            std::vector<double*> rho_Y;
            rho_Y.reserve(d_num_species);
            for (int si = 0; si < d_num_species; si++)
            {
                rho_Y.push_back(data_partial_density->getPointer(si));
            }
            double* E = data_total_energy->getPointer(0);
            double* p = d_data_pressure->getPointer(0);
            
            if (d_dim == tbox::Dimension(1))
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::"
                    << "computeGlobalCellDataConvectiveFluxWithVelocityAndPressure()\n"
                    << "'CONVECTIVE_FLUX_Y' cannot be obtained for problem with dimension less than two."
                    << std::endl);
            }
            else if (d_dim == tbox::Dimension(2))
            {
                // Get the pointers to the cell data of momentum.
                double* rho_u = data_momentum->getPointer(0);
                double* rho_v = data_momentum->getPointer(1);
                
                // Get the pointer to the cell data of velocity.
                double* v = d_data_velocity->getPointer(1);
                
                // Compute the convective flux in the y-direction.
                for (int j = -d_num_subghosts_convective_flux_y[1];
                     j < d_interior_dims[1] + d_num_subghosts_convective_flux_y[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_convective_flux_y[0];
                         i < d_interior_dims[0] + d_num_subghosts_convective_flux_y[0];
                         i++)
                    {
                        // Compute the linear indices.
                        const int idx = (i + d_num_ghosts[0]) +
                        (j + d_num_ghosts[1])*d_ghostcell_dims[0];
                        
                        const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                            (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0];
                        
                        const int idx_velocity = (i + d_num_subghosts_velocity[0]) +
                            (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_convective_flux_y = (i + d_num_subghosts_convective_flux_y[0]) +
                            (j + d_num_subghosts_convective_flux_y[1])*d_subghostcell_dims_convective_flux_y[0];
                        
                        for (int si = 0; si < d_num_species; si++)
                        {
                            F_y[si][idx_convective_flux_y] = v[idx_velocity]*rho_Y[si][idx];
                        }
                        F_y[d_num_species][idx_convective_flux_y] = v[idx_velocity]*rho_u[idx];
                        F_y[d_num_species + 1][idx_convective_flux_y] = v[idx_velocity]*rho_v[idx] + p[idx_pressure];
                        F_y[d_num_species + 2][idx_convective_flux_y] = v[idx_velocity]*(E[idx] + p[idx_pressure]);
                    }
                }
            }
            else if (d_dim == tbox::Dimension(3))
            {
                // Get the pointers to the cell data of momentum.
                double* rho_u = data_momentum->getPointer(0);
                double* rho_v = data_momentum->getPointer(1);
                double* rho_w = data_momentum->getPointer(2);
                
                // Get the pointer to the cell data of velocity.
                double* v = d_data_velocity->getPointer(1);
                
                // Compute the convective flux in the y-direction.
                for (int k = -d_num_subghosts_convective_flux_y[2];
                     k < d_interior_dims[2] + d_num_subghosts_convective_flux_y[2];
                     k++)
                {
                    for (int j = -d_num_subghosts_convective_flux_y[1];
                         j < d_interior_dims[1] + d_num_subghosts_convective_flux_y[1];
                         j++)
                    {
                        for (int i = -d_num_subghosts_convective_flux_y[0];
                             i < d_interior_dims[0] + d_num_subghosts_convective_flux_y[0];
                             i++)
                        {
                            // Compute the linear indices.
                            const int idx = (i + d_num_ghosts[0]) +
                                (j + d_num_ghosts[1])*d_ghostcell_dims[0] +
                                (k + d_num_ghosts[2])*d_ghostcell_dims[0]*d_ghostcell_dims[1];
                            
                            const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                                (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0] +
                                (k + d_num_subghosts_pressure[2])*d_subghostcell_dims_pressure[0]*
                                    d_subghostcell_dims_pressure[1];
                            
                            const int idx_velocity = (i + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_convective_flux_y = (i + d_num_subghosts_convective_flux_y[0]) +
                                (j + d_num_subghosts_convective_flux_y[1])*d_subghostcell_dims_convective_flux_y[0] +
                                (k + d_num_subghosts_convective_flux_y[2])*d_subghostcell_dims_convective_flux_y[0]*
                                    d_subghostcell_dims_convective_flux_y[1];
                            
                            for (int si = 0; si < d_num_species; si++)
                            {
                                F_y[si][idx_convective_flux_y] = v[idx_velocity]*rho_Y[si][idx];
                            }
                            F_y[d_num_species][idx_convective_flux_y] = v[idx_velocity]*rho_u[idx];
                            F_y[d_num_species + 1][idx_convective_flux_y] = v[idx_velocity]*rho_v[idx] + p[idx_pressure];
                            F_y[d_num_species + 2][idx_convective_flux_y] = v[idx_velocity]*rho_w[idx];
                            F_y[d_num_species + 3][idx_convective_flux_y] = v[idx_velocity]*(E[idx] + p[idx_pressure]);
                        }
                    }
                }
            }
        }
        else
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::"
                << "computeGlobalCellDataConvectiveFluxWithVelocityAndPressure()\n"
                << "Cell data of 'CONVECTIVE_FLUX_Y' is not yet registered."
                << std::endl);
        }
    }
    else if (direction == DIRECTION::Z_DIRECTION)
    {
        if (d_num_subghosts_convective_flux_z > -hier::IntVector::getOne(d_dim))
        {
            // Create the cell data of convective flux in the z-direction.
            d_data_convective_flux_z.reset(
                new pdat::CellData<double>(d_interior_box, d_num_eqn, d_num_subghosts_convective_flux_z));
            
            // Get the pointers to the components of the convective flux in the z-direction.
            std::vector<double*> F_z;
            F_z.reserve(d_num_eqn);
            for (int ei = 0; ei < d_num_eqn; ei++)
            {
                F_z.push_back(d_data_convective_flux_z->getPointer(ei));
            }
            
            boost::shared_ptr<pdat::CellData<double> > data_partial_density =
                getGlobalCellDataPartialDensity();
            
            boost::shared_ptr<pdat::CellData<double> > data_momentum =
                getGlobalCellDataMomentum();
            
            boost::shared_ptr<pdat::CellData<double> > data_total_energy =
                getGlobalCellDataTotalEnergy();
            
            if (!d_data_pressure)
            {
                computeGlobalCellDataPressureWithDensityMassFractionAndInternalEnergy();
            }
            
            if (!d_data_velocity)
            {
                computeGlobalCellDataVelocityWithDensity();
            }
            
            // Get the pointers to the cell data of partial density, total energy, volume fraction
            // and pressure.
            std::vector<double*> rho_Y;
            rho_Y.reserve(d_num_species);
            for (int si = 0; si < d_num_species; si++)
            {
                rho_Y.push_back(data_partial_density->getPointer(si));
            }
            double* E = data_total_energy->getPointer(0);
            double* p = d_data_pressure->getPointer(0);
            
            if (d_dim == tbox::Dimension(1) || d_dim == tbox::Dimension(2))
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::"
                    << "computeGlobalCellDataConvectiveFluxWithVelocityAndPressure()\n"
                    << "'CONVECTIVE_FLUX_Z' cannot be obtained for problem with dimension less than three."
                    << std::endl);
            }
            else if (d_dim == tbox::Dimension(3))
            {
                // Get the pointers to the cell data of momentum.
                double* rho_u = data_momentum->getPointer(0);
                double* rho_v = data_momentum->getPointer(1);
                double* rho_w = data_momentum->getPointer(2);
                
                // Get the pointer to the cell data of velocity.
                double* w = d_data_velocity->getPointer(2);
                
                // Compute the convective flux in the z-direction.
                for (int k = -d_num_subghosts_convective_flux_z[2];
                     k < d_interior_dims[2] + d_num_subghosts_convective_flux_z[2];
                     k++)
                {
                    for (int j = -d_num_subghosts_convective_flux_z[1];
                         j < d_interior_dims[1] + d_num_subghosts_convective_flux_z[1];
                         j++)
                    {
                        for (int i = -d_num_subghosts_convective_flux_z[0];
                             i < d_interior_dims[0] + d_num_subghosts_convective_flux_z[0];
                             i++)
                        {
                            // Compute the linear indices.
                            const int idx = (i + d_num_ghosts[0]) +
                                (j + d_num_ghosts[1])*d_ghostcell_dims[0] +
                                (k + d_num_ghosts[2])*d_ghostcell_dims[0]*d_ghostcell_dims[1];
                            
                            const int idx_pressure = (i + d_num_subghosts_pressure[0]) +
                                (j + d_num_subghosts_pressure[1])*d_subghostcell_dims_pressure[0] +
                                (k + d_num_subghosts_pressure[2])*d_subghostcell_dims_pressure[0]*
                                    d_subghostcell_dims_pressure[1];
                            
                            const int idx_velocity = (i + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_convective_flux_z = (i + d_num_subghosts_convective_flux_z[0]) +
                                (j + d_num_subghosts_convective_flux_z[1])*d_subghostcell_dims_convective_flux_z[0] +
                                (k + d_num_subghosts_convective_flux_z[2])*d_subghostcell_dims_convective_flux_z[0]*
                                    d_subghostcell_dims_convective_flux_z[1];
                            
                            for (int si = 0; si < d_num_species; si++)
                            {
                                F_z[si][idx_convective_flux_z] = w[idx_velocity]*rho_Y[si][idx];
                            }
                            F_z[d_num_species][idx_convective_flux_z] = w[idx_velocity]*rho_u[idx];
                            F_z[d_num_species + 1][idx_convective_flux_z] = w[idx_velocity]*rho_v[idx];
                            F_z[d_num_species + 2][idx_convective_flux_z] = w[idx_velocity]*rho_w[idx] + p[idx_pressure];
                            F_z[d_num_species + 3][idx_convective_flux_z] = w[idx_velocity]*(E[idx] + p[idx_pressure]);
                        }
                    }
                }
            }
        }
        else
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::"
                << "computeGlobalCellDataConvectiveFluxWithVelocityAndPressure()\n"
                << "Cell data of 'CONVECTIVE_FLUX_Z' is not yet registered."
                << std::endl);
        }
    }
}


/*
 * Compute the global cell data of maximum wave speed with velocity and sound speed in the
 * registered patch.
 */
void
FlowModelFourEqnConservative::computeGlobalCellDataMaxWaveSpeedWithVelocityAndSoundSpeed(
    DIRECTION::TYPE direction)
{
    if (direction == DIRECTION::X_DIRECTION)
    {
        if (d_num_subghosts_max_wave_speed_x > -hier::IntVector::getOne(d_dim))
        {
            // Create the cell data of maximum wave speed in the x-direction.
            d_data_max_wave_speed_x.reset(
                new pdat::CellData<double>(d_interior_box, 1, d_num_subghosts_max_wave_speed_x));
            
            if (!d_data_velocity)
            {
                computeGlobalCellDataVelocityWithDensity();
            }
            
            if (!d_data_sound_speed)
            {
                computeGlobalCellDataSoundSpeedWithDensityMassFractionAndPressure();
            }
            
            // Get the pointers to the cell data of maximum wave speed and velocity in x-direction and sound speed.
            double* lambda_max_x = d_data_max_wave_speed_x->getPointer(0);
            double* u            = d_data_velocity->getPointer(0);
            double* c            = d_data_sound_speed->getPointer(0);
            
            if (d_dim == tbox::Dimension(1))
            {
                // Compute the maximum wave speed in the x-direction.
                for (int i = -d_num_subghosts_max_wave_speed_x[0];
                     i < d_interior_dims[0] + d_num_subghosts_max_wave_speed_x[0];
                     i++)
                {
                    // Compute the linear indices.
                    const int idx_sound_speed = i + d_num_subghosts_sound_speed[0];
                    const int idx_velocity = i + d_num_subghosts_velocity[0];
                    const int idx_max_wave_speed_x = i + d_num_subghosts_max_wave_speed_x[0];
                    
                    lambda_max_x[idx_max_wave_speed_x] = fabs(u[idx_velocity]) + c[idx_sound_speed];
                }
            }
            else if (d_dim == tbox::Dimension(2))
            {
                // Compute the maximum wave speed in the x-direction.
                for (int j = -d_num_subghosts_max_wave_speed_x[1];
                     j < d_interior_dims[1] + d_num_subghosts_max_wave_speed_x[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_max_wave_speed_x[0];
                         i < d_interior_dims[0] + d_num_subghosts_max_wave_speed_x[0];
                         i++)
                    {
                        // Compute the linear indices.
                        const int idx_sound_speed = (i + d_num_subghosts_sound_speed[0]) +
                            (j + d_num_subghosts_sound_speed[1])*d_subghostcell_dims_sound_speed[0];
                        
                        const int idx_velocity = (i + d_num_subghosts_velocity[0]) +
                            (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_max_wave_speed_x = (i + d_num_subghosts_max_wave_speed_x[0]) +
                            (j + d_num_subghosts_max_wave_speed_x[1])*d_subghostcell_dims_max_wave_speed_x[0];
                        
                        lambda_max_x[idx_max_wave_speed_x] = fabs(u[idx_velocity]) + c[idx_sound_speed];
                    }
                }
            }
            else if (d_dim == tbox::Dimension(3))
            {
                // Compute the maximum wave speed in the x-direction.
                for (int k = -d_num_subghosts_max_wave_speed_x[2];
                     k < d_interior_dims[2] + d_num_subghosts_max_wave_speed_x[2];
                     k++)
                {
                    for (int j = -d_num_subghosts_max_wave_speed_x[1];
                         j < d_interior_dims[1] + d_num_subghosts_max_wave_speed_x[1];
                         j++)
                    {
                        for (int i = -d_num_subghosts_max_wave_speed_x[0];
                             i < d_interior_dims[0] + d_num_subghosts_max_wave_speed_x[0];
                             i++)
                        {
                            // Compute the linear indices.
                            const int idx_sound_speed = (i + d_num_subghosts_sound_speed[0]) +
                                (j + d_num_subghosts_sound_speed[1])*d_subghostcell_dims_sound_speed[0] +
                                (k + d_num_subghosts_sound_speed[2])*d_subghostcell_dims_sound_speed[0]*
                                    d_subghostcell_dims_sound_speed[1];
                            
                            const int idx_velocity = (i + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_max_wave_speed_x = (i + d_num_subghosts_max_wave_speed_x[0]) +
                                (j + d_num_subghosts_max_wave_speed_x[1])*d_subghostcell_dims_max_wave_speed_x[0] +
                                (k + d_num_subghosts_max_wave_speed_x[2])*d_subghostcell_dims_max_wave_speed_x[0]*
                                    d_subghostcell_dims_max_wave_speed_x[1];
                            
                            lambda_max_x[idx_max_wave_speed_x] = fabs(u[idx_velocity]) + c[idx_sound_speed];
                        }
                    }
                }
            }
        }
        else
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::"
                << "computeGlobalCellDataMaxWaveSpeedWithVelocityAndSoundSpeed()\n"
                << "Cell data of 'MAX_WAVE_SPEED_X' is not yet registered."
                << std::endl);
        }
    }
    else if (direction == DIRECTION::Y_DIRECTION)
    {
        if (d_num_subghosts_max_wave_speed_y > -hier::IntVector::getOne(d_dim))
        {
            // Create the cell data of maximum wave speed in the y-direction.
            d_data_max_wave_speed_y.reset(
                new pdat::CellData<double>(d_interior_box, 1, d_num_subghosts_max_wave_speed_y));
            
            if (!d_data_sound_speed)
            {
                computeGlobalCellDataSoundSpeedWithDensityMassFractionAndPressure();
            }
            
            if (!d_data_velocity)
            {
                computeGlobalCellDataVelocityWithDensity();
            }
            
            // Get the pointers to the cell data of maximum wave speed and velocity in y-direction, and sound speed.
            double* lambda_max_y = d_data_max_wave_speed_y->getPointer(0);
            double* v            = d_data_velocity->getPointer(1);
            double* c            = d_data_sound_speed->getPointer(0);
            
            if (d_dim == tbox::Dimension(1))
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::"
                    << "computeGlobalCellDataMaxWaveSpeedWithVelocityAndSoundSpeed()\n"
                    << "'MAX_WAVE_SPEED_Y' cannot be obtained for problem with dimension less than two."
                    << std::endl);
            }
            else if (d_dim == tbox::Dimension(2))
            {
                // Compute the maximum wave speed in the y-direction.
                for (int j = -d_num_subghosts_max_wave_speed_y[1];
                     j < d_interior_dims[1] + d_num_subghosts_max_wave_speed_y[1];
                     j++)
                {
                    for (int i = -d_num_subghosts_max_wave_speed_y[0];
                         i < d_interior_dims[0] + d_num_subghosts_max_wave_speed_y[0];
                         i++)
                    {
                        // Compute the linear indices.
                        const int idx_sound_speed = (i + d_num_subghosts_sound_speed[0]) +
                            (j + d_num_subghosts_sound_speed[1])*d_subghostcell_dims_sound_speed[0];
                        
                        const int idx_velocity = (i + d_num_subghosts_velocity[0]) +
                            (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0];
                        
                        const int idx_max_wave_speed_y = (i + d_num_subghosts_max_wave_speed_y[0]) +
                            (j + d_num_subghosts_max_wave_speed_y[1])*d_subghostcell_dims_max_wave_speed_y[0];
                        
                        lambda_max_y[idx_max_wave_speed_y] = fabs(v[idx_velocity]) + c[idx_sound_speed];
                    }
                }
            }
            else if (d_dim == tbox::Dimension(3))
            {
                // Compute the maximum wave speed in the y-direction.
                for (int k = -d_num_subghosts_max_wave_speed_y[2];
                     k < d_interior_dims[2] + d_num_subghosts_max_wave_speed_y[2];
                     k++)
                {
                    for (int j = -d_num_subghosts_max_wave_speed_y[1];
                         j < d_interior_dims[1] + d_num_subghosts_max_wave_speed_y[1];
                         j++)
                    {
                        for (int i = -d_num_subghosts_max_wave_speed_y[0];
                             i < d_interior_dims[0] + d_num_subghosts_max_wave_speed_y[0];
                             i++)
                        {
                            // Compute the linear indices.
                            const int idx_sound_speed = (i + d_num_subghosts_sound_speed[0]) +
                                (j + d_num_subghosts_sound_speed[1])*d_subghostcell_dims_sound_speed[0] +
                                (k + d_num_subghosts_sound_speed[2])*d_subghostcell_dims_sound_speed[0]*
                                    d_subghostcell_dims_sound_speed[1];
                            
                            const int idx_velocity = (i + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_max_wave_speed_y = (i + d_num_subghosts_max_wave_speed_y[0]) +
                                (j + d_num_subghosts_max_wave_speed_y[1])*d_subghostcell_dims_max_wave_speed_y[0] +
                                (k + d_num_subghosts_max_wave_speed_y[2])*d_subghostcell_dims_max_wave_speed_y[0]*
                                    d_subghostcell_dims_max_wave_speed_y[1];
                            
                            lambda_max_y[idx_max_wave_speed_y] = fabs(v[idx_velocity]) + c[idx_sound_speed];
                        }
                    }
                }
            }
        }
        else
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::"
                << "computeGlobalCellDataMaxWaveSpeedWithVelocityAndSoundSpeed()\n"
                << "Cell data of 'MAX_WAVE_SPEED_Y' is not yet registered."
                << std::endl);
        }
    }
    else if (direction == DIRECTION::Z_DIRECTION)
    {
        if (d_num_subghosts_max_wave_speed_z > -hier::IntVector::getOne(d_dim))
        {
            // Create the cell data of maximum wave speed in the z-direction.
            d_data_max_wave_speed_z.reset(
                new pdat::CellData<double>(d_interior_box, 1, d_num_subghosts_max_wave_speed_z));
            
            if (!d_data_sound_speed)
            {
                computeGlobalCellDataSoundSpeedWithDensityMassFractionAndPressure();
            }
            
            if (!d_data_velocity)
            {
                computeGlobalCellDataVelocityWithDensity();
            }
            
            // Get the pointers to the cell data of maximum wave speed and velocity in z-direction, and sound speed.
            double* lambda_max_z = d_data_max_wave_speed_z->getPointer(0);
            double* w            = d_data_velocity->getPointer(2);
            double* c            = d_data_sound_speed->getPointer(0);
            
            if (d_dim == tbox::Dimension(1) || d_dim == tbox::Dimension(2))
            {
                TBOX_ERROR(d_object_name
                    << ": FlowModelFourEqnConservative::"
                    << "computeGlobalCellDataMaxWaveSpeedWithVelocityAndSoundSpeed()\n"
                    << "'MAX_WAVE_SPEED_Z' cannot be obtained for problem with dimension less than three."
                    << std::endl);
            }
            else if (d_dim == tbox::Dimension(3))
            {
                // Compute the maximum wave speed in the z-direction.
                for (int k = -d_num_subghosts_max_wave_speed_z[2];
                     k < d_interior_dims[2] + d_num_subghosts_max_wave_speed_z[2];
                     k++)
                {
                    for (int j = -d_num_subghosts_max_wave_speed_z[1];
                         j < d_interior_dims[1] + d_num_subghosts_max_wave_speed_z[1];
                         j++)
                    {
                        for (int i = -d_num_subghosts_max_wave_speed_z[0];
                             i < d_interior_dims[0] + d_num_subghosts_max_wave_speed_z[0];
                             i++)
                        {
                            // Compute the linear indices.
                            const int idx_sound_speed = (i + d_num_subghosts_sound_speed[0]) +
                                (j + d_num_subghosts_sound_speed[1])*d_subghostcell_dims_sound_speed[0] +
                                (k + d_num_subghosts_sound_speed[2])*d_subghostcell_dims_sound_speed[0]*
                                    d_subghostcell_dims_sound_speed[1];
                            
                            const int idx_velocity = (i + d_num_subghosts_velocity[0]) +
                                (j + d_num_subghosts_velocity[1])*d_subghostcell_dims_velocity[0] +
                                (k + d_num_subghosts_velocity[2])*d_subghostcell_dims_velocity[0]*
                                    d_subghostcell_dims_velocity[1];
                            
                            const int idx_max_wave_speed_z = (i + d_num_subghosts_max_wave_speed_z[0]) +
                                (j + d_num_subghosts_max_wave_speed_z[1])*d_subghostcell_dims_max_wave_speed_z[0] +
                                (k + d_num_subghosts_max_wave_speed_z[2])*d_subghostcell_dims_max_wave_speed_z[0]*
                                    d_subghostcell_dims_max_wave_speed_z[1];
                            
                            lambda_max_z[idx_max_wave_speed_z] = fabs(w[idx_velocity]) + c[idx_sound_speed];
                        }
                    }
                }
            }
        }
        else
        {
            TBOX_ERROR(d_object_name
                << ": FlowModelFourEqnConservative::"
                << "computeGlobalCellDataMaxWaveSpeedWithVelocityAndSoundSpeed()\n"
                << "Cell data of 'MAX_WAVE_SPEED_Z' is not yet registered."
                << std::endl);
        }
    }
}